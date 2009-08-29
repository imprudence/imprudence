/** 
 * @file vorbisdecode.cpp
 * @brief Vorbis decoding routine routine for Indra.
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "linden_common.h"

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "llerror.h"
#include "llmath.h"
#include "llvfile.h"

#if 0

size_t vfs_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	LLVFile *file = (LLVFile *)datasource;

	if (size > 0 && file->read((U8*)ptr, size * nmemb))	/*Flawfinder: ignore*/
	{
		S32 read = file->getLastBytesRead();
		return  read / size;	/*Flawfinder: ignore*/
	}
	else
	{
		return 0;
	}
}

int vfs_seek(void *datasource, ogg_int64_t offset, int whence)
{
	LLVFile *file = (LLVFile *)datasource;

	// vfs has 31-bit files
	if (offset > S32_MAX)
	{
		return -1;
	}

	S32 origin;
	switch (whence) {
	case SEEK_SET:
		origin = 0;
		break;
	case SEEK_END:
		origin = file->getSize();
		break;
	case SEEK_CUR:
		origin = -1;
		break;
	default:
		llerrs << "Invalid whence argument to vfs_seek" << llendl;
		return -1;
	}

	if (file->seek((S32)offset, origin))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int vfs_close (void *datasource)
{
	LLVFile *file = (LLVFile *)datasource;

	delete file;

	return 0;
}

long vfs_tell (void *datasource)
{
	LLVFile *file = (LLVFile *)datasource;

	return file->tell();
}


BOOL decode_vorbis_file(LLVFS *vfs, const LLUUID &in_uuid, char *out_fname)
{
	ov_callbacks vfs_callbacks;
	vfs_callbacks.read_func = vfs_read;
	vfs_callbacks.seek_func = vfs_seek;
	vfs_callbacks.close_func = vfs_close;
	vfs_callbacks.tell_func = vfs_tell;

	char pcmout[4096]; /*Flawfinder: ignore*/

	unsigned char temp[64];	/*Flawfinder: ignore*/

	LLVFile *in_vfile;

	U32 data_length = 0;

	llinfos << "Vorbis decode from vfile: " << in_uuid << llendl;

	in_vfile = new LLVFile(vfs, in_uuid, LLAssetType::AT_SOUND);
	if (! in_vfile->getSize())
	{
		llwarning("unable to open vorbis source vfile for reading",0);
		return(FALSE);
	}

	//**********************************
	LLAPRFile outfile ;
	outfile.open(out_fname,LL_APR_WPB);
	//**********************************
	if (!outfile.getFileHandle())
	{
		llwarning("unable to open vorbis destination file for writing",0);
		return(FALSE);		
	}
	else
	{
		// write the .wav format header
		//"RIFF"
		temp[0] = 0x52;
		temp[1] = 0x49;
		temp[2] = 0x46;
		temp[3] = 0x46;
		
		// length = datalen + 36 (to be filled in later)
		temp[4] = 0x00;
		temp[5] = 0x00;
		temp[6] = 0x00;
		temp[7] = 0x00;
		
		//"WAVE"
		temp[8] = 0x57;
		temp[9] = 0x41;
		temp[10] = 0x56;
		temp[11] = 0x45;

		// "fmt "
		temp[12] = 0x66;
		temp[13] = 0x6D;
		temp[14] = 0x74;
		temp[15] = 0x20;

		// chunk size = 16
		temp[16] = 0x10;
		temp[17] = 0x00;
		temp[18] = 0x00;
		temp[19] = 0x00;

		// format (1 = PCM)
		temp[20] = 0x01;
		temp[21] = 0x00;

		// number of channels
		temp[22] = 0x01;
		temp[23] = 0x00;

		// samples per second
		temp[24] = 0x44;
		temp[25] = 0xAC;
		temp[26] = 0x00;
		temp[27] = 0x00;

		// average bytes per second
		temp[28] = 0x88;
		temp[29] = 0x58;
		temp[30] = 0x01;
		temp[31] = 0x00;

		// bytes to output at a single time
		temp[32] = 0x02;
		temp[33] = 0x00;
		 
		// 16 bits per sample
		temp[34] = 0x10;
		temp[35] = 0x00;

		// "data"
		temp[36] = 0x64;
		temp[37] = 0x61;
		temp[38] = 0x74;
		temp[39] = 0x61;


		// these are the length of the data chunk, to be filled in later
		temp[40] = 0x00;
		temp[41] = 0x00;
		temp[42] = 0x00;
		temp[43] = 0x00;

		outfile.write(temp, 44);
	}

	OggVorbis_File vf;
	int eof=0;
	int current_section;
 
	int r = ov_open_callbacks(in_vfile, &vf, NULL, 0, vfs_callbacks);
	if(r < 0) 
	{
		llwarns << r << " Input to vorbis decode does not appear to be an Ogg bitstream: " << in_uuid << llendl;		
		return(FALSE);
	}

	{
		char **ptr=ov_comment(&vf,-1)->user_comments;
//		vorbis_info *vi=ov_info(&vf,-1);
		while(*ptr){
			fprintf(stderr,"%s\n",*ptr);
			++ptr;
		}
//    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
//    fprintf(stderr,"\nDecoded length: %ld samples\n", (long)ov_pcm_total(&vf,-1));
//    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
	}
  
	while(!eof){
		long ret=ov_read(&vf,pcmout,sizeof(pcmout),0,2,1,&current_section);
		if (ret == 0) {
			/* EOF */
			eof=1;
//			llinfos << "Vorbis EOF" << llendl;
		} else if (ret < 0) {
			/* error in the stream.  Not a problem, just reporting it in
			   case we (the app) cares.  In this case, we don't. */
			llwarning("Error in vorbis stream",0); 
			break;

		} else {
//			llinfos << "Vorbis read " << ret << "bytes" << llendl;
			/* we don't bother dealing with sample rate changes, etc, but.
			   you'll have to*/
			data_length += outfile.write(pcmout, ret);
		}
	}

	ov_clear(&vf);
  
	// write "data" chunk length
	outfile.seek(APR_SET,40);
	outfile.write(&data_length,4);
 
	// write overall "RIFF" length
	data_length += 36;
	outfile.seek(APR_SET,4);
	outfile.write(&data_length,1*4);

	// FUCK!!! Vorbis encode/decode messes up loop point transitions (pop)
	// do a cheap-and-cheesy crossfade 
	

	S16 *samplep;
	S32 i;
	S32 fade_length;

	fade_length = llmin((S32)128,(S32)(data_length-36)/8);
	
	outfile.seek(APR_SET,44);
	outfile.read(pcmout,2*fade_length);  //read first 16 samples

	samplep = (S16 *)pcmout;

	for (i = 0 ;i < fade_length; i++)
	{
		*samplep++ = ((F32)*samplep * ((F32)i/(F32)fade_length));	   
	}
	
	outfile.seek(APR_SET,44);
	outfile.write(pcmout,2*fade_length);  //write back xfaded first 16 samples

	outfile.seek(APR_END,-fade_length*2); 
	outfile.read(pcmout,2*fade_length);  //read last 16 samples

	samplep = (S16 *)pcmout;

	for (i = fade_length-1 ; i >=  0; i--)
	{
		*samplep++ = ((F32)*samplep * ((F32)i/(F32)fade_length));	   
	}
	
	outfile.seek(SEEK_END,-fade_length*2); 
	outfile.write(pcmout,2*fade_length);  //write back xfaded last 16 samples
	//*******************
	outfile.close();
	//*******************
	
	if ((36 == data_length) || (!(eof)))
	{
		llwarning("BAD Vorbis DECODE!, removing .wav!",0); 
		LLFile::remove(out_fname);
		return (FALSE);
	}
    
//	fprintf(stderr,"Done.\n");

	return(TRUE);
}
#endif
