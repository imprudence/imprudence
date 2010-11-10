/**
 * @file aithreadsafe.h
 * @brief Implementation of AIThreadSafe, AIReadAccessConst, AIReadAccess and AIWriteAccess.
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   31/03/2010
 *   Initial version, written by Aleric Inglewood @ SL
 */

#ifndef AITHREADSAFE_H
#define AITHREADSAFE_H

#include <new>

#include "llthread.h"
#include "llerror.h"

template<typename T> struct AIReadAccessConst;
template<typename T> struct AIReadAccess;
template<typename T> struct AIWriteAccess;
template<typename T> struct AIAccess;

template<typename T>
class AIThreadSafeBits
{
private:
	// AIThreadSafe is a wrapper around an instance of T.
	// Because T might not have a default constructor, it is constructed
	// 'in place', with placement new, in the memory reserved here.
	//
	// Make sure that the memory that T will be placed in is properly
	// aligned by using an array of long's.
	long mMemory[(sizeof(T) + sizeof(long) - 1) / sizeof(long)];

public:
	// The wrapped objects are constructed in-place with placement new *outside*
	// of this object (by AITHREADSAFE macro(s) or derived classes).
	// However, we are responsible for the destruction of the wrapped object.
	~AIThreadSafeBits() { ptr()->~T(); }

	// Only for use by AITHREADSAFE, see below.
	void* memory() const { return const_cast<long*>(&mMemory[0]); }

protected:
	// Accessors.
	T const* ptr() const { return reinterpret_cast<T const*>(mMemory); }
	T* ptr() { return reinterpret_cast<T*>(mMemory); }
};

/**
 * @brief A wrapper class for objects that need to be accessed by more than one thread, allowing concurrent readers.
 *
 * Use AITHREADSAFE to define instances of any type, and use AIReadAccessConst,
 * AIReadAccess and AIWriteAccess to get access to the instance.
 *
 * For example,
 *
 * <code>
 * class Foo { public: Foo(int, int); };
 *
 * AITHREADSAFE(Foo, foo, (2, 3));
 *
 * AIReadAccess<Foo> foo_r(foo);
 * // Use foo_r-> for read access.
 *
 * AIWriteAccess<Foo> foo_w(foo);
 * // Use foo_w-> for write access.
 * </code>
 *
 * If <code>foo</code> is constant, you have to use <code>AIReadAccessConst<Foo></code>.
 *
 * It is possible to pass access objects to a function that
 * downgrades the access, for example:
 *
 * <code>
 * void readfunc(AIReadAccess const& access);
 *
 * AIWriteAccess<Foo> foo_w(foo);
 * readfunc(foo_w);					// readfunc will perform read access to foo_w.
 * </code>
 *
 * If <code>AIReadAccess</code> is non-const, you can upgrade the access by creating
 * an <code>AIWriteAccess</code> object from it. For example:
 *
 * <code>
 * AIWriteAccess<Foo> foo_w(foo_r);
 * </code>
 *
 * This API is Robust(tm). If you try anything that could result in problems,
 * it simply won't compile. The only mistake you can still easily make is
 * to obtain write access to an object when it is not needed, or to unlock
 * an object in between accesses while the state of the object should be
 * preserved. For example:
 *
 * <code>
 * // This resets foo to point to the first file and then returns that.
 * std::string filename = AIWriteAccess<Foo>(foo)->get_first_filename();
 *
 * // WRONG! The state between calling get_first_filename and get_next_filename should be preserved!
 *
 * AIWriteAccess<Foo> foo_w(foo);	// Wrong. The code below only needs read-access.
 * while (!filename.empty())
 * {
 *     something(filename);
 *     filename = foo_w->next_filename();
 * }
 * </code>
 *
 * Correct would be
 *
 * <code>
 * AIReadAccess<Foo> foo_r(foo);
 * std::string filename = AIWriteAccess<Foo>(foo_r)->get_first_filename();
 * while (!filename.empty())
 * {
 *     something(filename);
 *     filename = foo_r->next_filename();
 * }
 * </code>
 *
 */
template<typename T>
class AIThreadSafe : public AIThreadSafeBits<T>
{
protected:
	// Only these may access the object (through ptr()).
	friend struct AIReadAccessConst<T>;
	friend struct AIReadAccess<T>;
	friend struct AIWriteAccess<T>;

	// Locking control.
	AIRWLock mRWLock;

	// For use by AIThreadSafeDC
	AIThreadSafe(void) { }
	AIThreadSafe(AIAPRPool& parent) : mRWLock(parent) { }

public:
	// Only for use by AITHREADSAFE, see below.
	AIThreadSafe(T* object) { llassert(object == AIThreadSafeBits<T>::ptr()); }
};

/**
 * @brief Instantiate an static, global or local object of a given type wrapped in AIThreadSafe, using an arbitrary constructor.
 *
 * For example, instead of doing
 *
 * <code>
 * Foo foo(x, y);
 * static Bar bar;
 * </code>
 *
 * One can instantiate a thread-safe instance with
 *
 * <code>
 * AITHREADSAFE(Foo, foo, (x, y));
 * static AITHREADSAFE(Bar, bar, );
 * </code>
 *
 * Note: This macro does not allow to allocate such object on the heap.
 *       If that is needed, have a look at AIThreadSafeDC.
 */
#define AITHREADSAFE(type, var, paramlist) AIThreadSafe<type> var(new (var.memory()) type paramlist)

/**
 * @brief A wrapper class for objects that need to be accessed by more than one thread.
 *
 * This class is the same as an AIThreadSafe wrapper, except that it can only
 * be used for default constructed objects.
 *
 * For example, instead of
 *
 * <code>
 * Foo foo;
 * </code>
 *
 * One would use
 *
 * <code>
 * AIThreadSafeDC<Foo> foo;
 * </code>
 *
 * The advantage over AITHREADSAFE is that this object can be allocated with
 * new on the heap. For example:
 *
 * <code>
 * AIThreadSafeDC<Foo>* ptr = new AIThreadSafeDC<Foo>;
 * </code>
 *
 * which is not possible with AITHREADSAFE.
 */
template<typename T>
class AIThreadSafeDC : public AIThreadSafe<T>
{
public:
	// Construct a wrapper around a default constructed object.
	AIThreadSafeDC(void) { new (AIThreadSafe<T>::ptr()) T; }
};

/**
 * @brief Read lock object and provide read access.
 */
template<typename T>
struct AIReadAccessConst
{
	//! Internal enum for the lock-type of the AI*Access object.
	enum state_type
	{
		readlocked,			//!< A AIReadAccessConst or AIReadAccess.
		read2writelocked,	//!< A AIWriteAccess constructed from a AIReadAccess.
		writelocked,		//!< A AIWriteAccess constructed from a AIThreadSafe.
		write2writelocked	//!< A AIWriteAccess constructed from (the AIReadAccess base class of) a AIWriteAccess.
	};

	//! Construct a AIReadAccessConst from a constant AIThreadSafe.
	AIReadAccessConst(AIThreadSafe<T> const& wrapper)
		: mWrapper(const_cast<AIThreadSafe<T>&>(wrapper)),
		  mState(readlocked)
		{
			mWrapper.mRWLock.rdlock();
		}

	//! Destruct the AI*Access object.
	// These should never be dynamically allocated, so there is no need to make this virtual.
	~AIReadAccessConst()
		{
			if (mState == readlocked)
				mWrapper.mRWLock.rdunlock();
			else if (mState == writelocked)
				mWrapper.mRWLock.wrunlock();
			else if (mState == read2writelocked)
				mWrapper.mRWLock.wr2rdlock();
		}

	//! Access the underlaying object for read access.
	T const* operator->() const { return mWrapper.ptr(); }

	//! Access the underlaying object for read access.
	T const& operator*() const { return *mWrapper.ptr(); }

protected:
	//! Constructor used by AIReadAccess.
	AIReadAccessConst(AIThreadSafe<T>& wrapper, state_type state)
		: mWrapper(wrapper), mState(state) { }

	AIThreadSafe<T>& mWrapper;	//!< Reference to the object that we provide access to.
	state_type const mState;	//!< The lock state that mWrapper is in.

private:
	// Disallow copy constructing directly.
	AIReadAccessConst(AIReadAccessConst const&);
};

/**
 * @brief Read lock object and provide read access, with possible promotion to write access.
 */
template<typename T>
struct AIReadAccess : public AIReadAccessConst<T>
{
	typedef typename AIReadAccessConst<T>::state_type state_type;
	using AIReadAccessConst<T>::readlocked;

	//! Construct a AIReadAccess from a non-constant AIThreadSafe.
	AIReadAccess(AIThreadSafe<T>& wrapper) : AIReadAccessConst<T>(wrapper, readlocked) { this->mWrapper.mRWLock.rdlock(); }

protected:
	//! Constructor used by AIWriteAccess.
	AIReadAccess(AIThreadSafe<T>& wrapper, state_type state) : AIReadAccessConst<T>(wrapper, state) { }

	friend class AIWriteAccess<T>;
};

/**
 * @brief Write lock object and provide read/write access.
 */
template<typename T>
struct AIWriteAccess : public AIReadAccess<T>
{
	using AIReadAccessConst<T>::readlocked;
	using AIReadAccessConst<T>::read2writelocked;
	using AIReadAccessConst<T>::writelocked;
	using AIReadAccessConst<T>::write2writelocked;

	//! Construct a AIWriteAccess from a non-constant AIThreadSafe.
	AIWriteAccess(AIThreadSafe<T>& wrapper) : AIReadAccess<T>(wrapper, writelocked) { this->mWrapper.mRWLock.wrlock();}

	//! Promote read access to write access.
	explicit AIWriteAccess(AIReadAccess<T>& access)
		: AIReadAccess<T>(access.mWrapper, (access.mState == readlocked) ? read2writelocked : write2writelocked)
		{
			if (this->mState == read2writelocked)
			{
				this->mWrapper.mRWLock.rd2wrlock();
			}
		}

	//! Access the underlaying object for (read and) write access.
	T* operator->() const { return this->mWrapper.ptr(); }

	//! Access the underlaying object for (read and) write access.
	T& operator*() const { return *this->mWrapper.ptr(); }
};

/**
 * @brief A wrapper class for objects that need to be accessed by more than one thread.
 *
 * Use AITHREADSAFESIMPLE to define instances of any type, and use AIAccess
 * to get access to the instance.
 *
 * For example,
 *
 * <code>
 * class Foo { public: Foo(int, int); };
 *
 * AITHREADSAFESIMPLE(Foo, foo, (2, 3));
 *
 * AIAccess<Foo> foo_w(foo);
 * // Use foo_w-> for read and write access.
 *
 * See also AIThreadSafe
 */
template<typename T>
class AIThreadSafeSimple : public AIThreadSafeBits<T>
{
protected:
	// Only this one may access the object (through ptr()).
	friend struct AIAccess<T>;

	// Locking control.
	LLMutex mMutex;

	// For use by AIThreadSafeSimpleDC
	AIThreadSafeSimple(void) { }
	AIThreadSafeSimple(AIAPRPool& parent) : mMutex(parent) { }

public:
	// Only for use by AITHREADSAFESIMPLE, see below.
	AIThreadSafeSimple(T* object) { llassert(object == AIThreadSafeBits<T>::ptr()); }
};

/**
 * @brief Instantiate an static, global or local object of a given type wrapped in AIThreadSafeSimple, using an arbitrary constructor.
 *
 * For example, instead of doing
 *
 * <code>
 * Foo foo(x, y);
 * static Bar bar;
 * </code>
 *
 * One can instantiate a thread-safe instance with
 *
 * <code>
 * AITHREADSAFESIMPLE(Foo, foo, (x, y));
 * static AITHREADSAFESIMPLE(Bar, bar, );
 * </code>
 *
 * Note: This macro does not allow to allocate such object on the heap.
 *       If that is needed, have a look at AIThreadSafeSimpleDC.
 */
#define AITHREADSAFESIMPLE(type, var, paramlist) AIThreadSafeSimple<type> var(new (var.memory()) type paramlist)

/**
 * @brief A wrapper class for objects that need to be accessed by more than one thread.
 *
 * This class is the same as an AIThreadSafeSimple wrapper, except that it can only
 * be used for default constructed objects.
 *
 * For example, instead of
 *
 * <code>
 * Foo foo;
 * </code>
 *
 * One would use
 *
 * <code>
 * AIThreadSafeSimpleDC<Foo> foo;
 * </code>
 *
 * The advantage over AITHREADSAFESIMPLE is that this object can be allocated with
 * new on the heap. For example:
 *
 * <code>
 * AIThreadSafeSimpleDC<Foo>* ptr = new AIThreadSafeSimpleDC<Foo>;
 * </code>
 *
 * which is not possible with AITHREADSAFESIMPLE.
 */
template<typename T>
class AIThreadSafeSimpleDC : public AIThreadSafeSimple<T>
{
public:
	// Construct a wrapper around a default constructed object.
	AIThreadSafeSimpleDC(void) { new (AIThreadSafeSimple<T>::ptr()) T; }

protected:
	// For use by AIThreadSafeSimpleDCRootPool
	AIThreadSafeSimpleDC(AIAPRPool& parent) : AIThreadSafeSimple<T>(parent) { new (AIThreadSafeSimple<T>::ptr()) T; }
};

// Helper class for AIThreadSafeSimpleDCRootPool to assure initialization of
// the root pool before constructing AIThreadSafeSimpleDC.
class AIThreadSafeSimpleDCRootPool_pbase
{
protected:
	AIAPRRootPool mRootPool;

private:
	template<typename T> friend class AIThreadSafeSimpleDCRootPool;
	AIThreadSafeSimpleDCRootPool_pbase(void) { }
};

/**
 * @brief A wrapper class for objects that need to be accessed by more than one thread.
 *
 * The same as AIThreadSafeSimpleDC except that this class creates its own AIAPRRootPool
 * for the internally used mutexes and condition, instead of using the current threads
 * root pool. The advantage of this is that it can be used for objects that need to
 * be accessed from the destructors of global objects (after main). The disadvantage
 * is that it's less efficient to use your own root pool, therefore it's use should be
 * restricted to those cases where it is absolutely necessary.
 */
template<typename T>
class AIThreadSafeSimpleDCRootPool : private AIThreadSafeSimpleDCRootPool_pbase, public AIThreadSafeSimpleDC<T>
{
public:
	// Construct a wrapper around a default constructed object, using memory allocated
	// from the operating system for the internal APR objects (mutexes and conditional),
	// as opposed to allocated from the current threads root pool.
	AIThreadSafeSimpleDCRootPool(void) :
		AIThreadSafeSimpleDCRootPool_pbase(),
		AIThreadSafeSimpleDC<T>(mRootPool) { }
};

/**
 * @brief Write lock object and provide read/write access.
 */
template<typename T>
struct AIAccess
{
	//! Construct a AIAccess from a non-constant AIThreadSafeSimple.
	AIAccess(AIThreadSafeSimple<T>& wrapper) : mWrapper(wrapper) { this->mWrapper.mMutex.lock(); }

	//! Access the underlaying object for (read and) write access.
	T* operator->() const { return this->mWrapper.ptr(); }

	//! Access the underlaying object for (read and) write access.
	T& operator*() const { return *this->mWrapper.ptr(); }

	~AIAccess() { this->mWrapper.mMutex.unlock(); }

protected:
	AIThreadSafeSimple<T>& mWrapper;	//!< Reference to the object that we provide access to.

private:
	// Disallow copy constructing directly.
	AIAccess(AIAccess const&);
};

#endif
