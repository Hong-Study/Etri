#pragma once
#include <memory>

using std::vector;
using std::map;
using std::shared_ptr;
using std::make_shared;
using std::wstring;
using std::to_wstring;
using std::atomic;


using SessionRef		= std::shared_ptr<class Session>;
using TifdRef			= std::shared_ptr<class TifdSession>;
using TirdRef			= std::shared_ptr<class TirdSession>;
using PairSessionRef	= std::shared_ptr<class PairSession>;
using SendBufferRef		= std::shared_ptr<class SendBuffer>;
using TifdListPtr		= std::shared_ptr<struct PendingTifdListViewItem>;
using TirdListPtr		= std::shared_ptr<struct PendingTirdListViewItem>;
using PListPtr			= std::shared_ptr<struct PairingListViewItem>;

#define BUFSIZE			512

#define USE_MANY_LOCK(idx)	Lock lock[idx]
#define USE_LOCK			USE_MANY_LOCK(1)
#define WRITE_LOCK_IDX(idx)	WriteLockGuard writeGuard_##idx(lock[idx], typeid(this).name())
#define WRITE_LOCK			WRITE_LOCK_IDX(0)
#define READ_LOCK_IDX(idx)	ReadLockGuard readGuard_##idx(lock[idx]);
#define READ_LOCK			READ_LOCK_IDX(0)

#define SINGLETON(classname)							\
private:												\
	classname() { }										\
	classname(const classname& ref) { }					\
	classname& operator=(const classname& ref) { }		\
	~classname() { }									\
public:													\
	static classname* GetInstance()						\
	{													\
		static classname instance;						\
		return &instance;								\
	}		

#define CRASH(cause)						\
{											\
	WINGUI->SaveLogData();					\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)			\
{									\
	if (!(expr))					\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}

#define GETSINGLE(classname) classname::GetInstance()

#define SERVER	GETSINGLE(AcceptServer)
#define THREAD	GETSINGLE(ThreadManager)
#define MAP		GETSINGLE(PythonMap)