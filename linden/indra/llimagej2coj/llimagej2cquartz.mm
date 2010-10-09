#import <Accelerate/Accelerate.h>
#import <QuartzCore/QuartzCore.h>
#import <Quartz/Quartz.h>

#include "llimagej2cquartz.h"

#if defined(__BIG_ENDIAN__)
	CGImageAlphaInfo const kDefaultAlphaLocation = kCGImageAlphaPremultipliedLast;
#else
	CGImageAlphaInfo const kDefaultAlphaLocation = kCGImageAlphaPremultipliedFirst;
#endif

BOOL decodeJ2CQuartz(LLImageJ2C &base, LLImageRaw &raw_image, F32 decode_time, S32 first_channel, S32 max_channel_count)
{
	U8 *srcData = (U8*)base.getData();
	int srcLen = base.getDataSize();

	llinfos << "[1] compressed image size: '" << srcLen << "'" << llendl;

	int width = base.getWidth();
	int height = base.getHeight();
	int components = base.getComponents();

	S32 channels = components - first_channel;
	if( channels > max_channel_count )
		channels = max_channel_count;

	llinfos << "[2] components: '" << components << "' - channels: '" << channels << "' - first_channel: '" << first_channel << "' - max_channel_count: '" << max_channel_count << "'" << llendl;

	if(components <= first_channel || components > 4)
		return FALSE;

	llinfos << "[3] attempting to decode a texture: '" << width << "'X'" << height << "'@'" << components * 8 << "'" << llendl;

	U8 *tgt = (U8*)raw_image.getData();
	if (!tgt)
		return FALSE;

	raw_image.resize(width, height, channels);

	size_t rowBytes = width * components;
	int realLen = width * height * components;

	llinfos << "[4] allocating buffer of size: '" << realLen << "' to hold temp texture data" << llendl;
	unsigned char* dataplane;

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSBitmapImageRep *rep = [NSBitmapImageRep alloc];

	switch (components)
	{
		case 1:
		{
			rep = [[rep
				   initWithBitmapDataPlanes:nil
				   pixelsWide:width
				   pixelsHigh:height
				   bitsPerSample:8
				   samplesPerPixel:1
				   hasAlpha:NO
				   isPlanar:NO
				   colorSpaceName:NSDeviceWhiteColorSpace
				   bytesPerRow:rowBytes
				   bitsPerPixel:8
				   ] autorelease];

			memcpy([rep bitmapData], srcData, srcLen);

			dataplane = (unsigned char*)malloc(realLen);
			memcpy(dataplane, [rep bitmapData], realLen);

			[rep release];
		}
		break;

		case 3:
		{
			NSData *data = [NSData dataWithBytes:srcData length:srcLen];
			rep = [rep initWithData:data];

			dataplane = (unsigned char*)malloc(realLen);
			memcpy(dataplane, [rep bitmapData], realLen);

			[data release];
			[rep release];
		}
		break;

		case 4:
		{
			NSData *data = [NSData dataWithBytes:srcData length:srcLen];
			rep = [rep initWithData:data];

			int imgLen = [rep pixelsHigh] * [rep bytesPerRow];
			if (imgLen != realLen)
			{
				llwarns << "decoded image buffer size (" << imgLen << ") != expected buffer size (" << realLen << ") !" << llendl;
				[rep release];
				[data release];
				return FALSE;
			}
			
			dataplane = (unsigned char*)malloc(realLen);
			memcpy(dataplane, [rep bitmapData], realLen);

			vImage_Buffer vb;
			vb.data = dataplane;
			vb.height = [rep pixelsHigh];
			vb.width = [rep pixelsWide];
			vb.rowBytes = [rep bytesPerRow];

			llinfos << "Attempting Alpha Unpremultiplication" << llendl;
			vImageUnpremultiplyData_RGBA8888(&vb, &vb, 0);
			llinfos << "Unpremultiplied Alpha" << llendl;

			llwarns << "after decoding: " << [rep pixelsWide] << "'X'" << [rep pixelsHigh] << "'@'" << [rep bitsPerPixel] << "'" << llendl;

			[rep release];
			[data release];
		}
		break;
	}

	if (dataplane)
	{
		for (int h=height-1; h>=0; h--)
		{
			for (int w=0; w<rowBytes; w+=(first_channel + channels))
			{
				for (int c=first_channel; c<(first_channel + channels); c++)
					memcpy(tgt++, &dataplane[h*rowBytes + w + c], sizeof(unsigned char));
			}
		}

		free(dataplane);

		llinfos << "[5] size of decoded image is: '" << width*height*channels << "'" << llendl;

		return TRUE;
	}
	else
	{
		llwarns << "[5] cannot decode image !" << llendl;
	}

	return FALSE;
}
