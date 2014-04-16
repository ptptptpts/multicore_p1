
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/time.h>

//#define __TESTINPUT
//#define __NOOUTPUT

//#define __TESTMUTEX
//#define __TESTBASIC
//#define __TESTCNT

#define __SPIN
//#define __MUTEX
//#define __COND

//#define __THREADMAX 4
#define __THREADDIV 10


//===================================
// macro function
//===================================
#define __OFFSET_Y			3
#define __OFFSET_Z			3 * _iExMapSize

#define __OFFSET_3D(x, y, z)	(((z) * _iExMapSize + (y)) * _iExMapSize + (x))
#define __OFFSET_2D(y,z) 		(((z) * _iExMapSize + (y)) * _iExMapSize)
#define __OFFSET_1D(z)		((z) * _iExMapSize * _iExMapSize)
//===================================


//===================================
// Function Declaration
//===================================
void init (void);
void ThreadInit (void);
void *ChildMain (void * argc);
void LifeGame(void);
void Out (void);

void Terminate (void);

// Game of Life Function
int SearchMap (void);
int CalcDoA (int x, int y, int z);
void MakeChange (int x, int y, int z);

//===================================


//===================================
// Global Variable
//===================================

// Initialize Value
int _iMapSize = 0;
int _iD1 = 0;
int _iD2 = 0;
int _iL1 = 0;
int _iL2 = 0;
int _iSteps = 0;
int __THREADMAX;

// Thread Value
int _nThreads;
pthread_t * _pThread;
char * _pMapCheck;
int _CheckSize;
int * _ThreadCnt;

// Thread Synchronization Value
int _iReady;
int _iFinParent;
pthread_mutex_t _mEndCnt;
int _iEndCnt;
int _iEndSign;
int _MapCheckLock = 0;
pthread_mutex_t _mMapCheck;
char _cLoopCheck;

// Map
int _iExMapSize;
char * _pExpandedMap;
char * _pNExpandedMap;

char * _pMap;
char * _pNMap;
char * _pNCMap;

char * _ppMap;
char * _ppNMap;
char * _ppCMap;
char * _ppNCMap;

// Changed Cell Map
char * _pCMap;

int _iChangeCnt = 0;

//===================================


//===================================
// Function Start
//===================================
int main (void)
{
	init ();
	ThreadInit();
	LifeGame();
#ifndef __NOOUTPUT
	Out ();
#endif
	
	Terminate ();
	
	return 0;
}


// 입력파일을 받아 Life Game을 초기화 시킨다
void init (void)
{
	int i, j, k;
	char *pMap, *pCMap;
	char *str, *str2, *tok;
	char *pNCMap;
	
	
	str = malloc (100);
	
	fgets(str, 100, stdin);
	
#ifdef __TESTINPUT
	printf("input file :: %s\n", str);
#endif
	
	str2 = strtok_r (str, " \n", &tok);
	_iMapSize = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	_iD1 = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	_iD2 = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	_iL1 = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	_iL2 = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	_iSteps = atoi (str2);
	
	str2 = strtok_r (NULL, " \n", &tok);
	__THREADMAX = atoi (str2);

	free (str);

#ifdef __TESTINPUT
	printf ("%d %d %d %d %d %d\n", _iMapSize, _iD1, _iD2, _iL1, _iL2, _iSteps);
#endif
	
	// Create Map
	_iExMapSize = _iMapSize + 3;
	
	str = malloc (sizeof (char) * _iMapSize * 3);
	_pExpandedMap = malloc (sizeof(char) * _iExMapSize * _iExMapSize * _iExMapSize);
	_pNExpandedMap =  malloc (sizeof(char) * _iExMapSize * _iExMapSize * _iExMapSize);
	_pCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	_pNCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);	
		
	// initialize Expanded Map
	pMap = _pExpandedMap;
	pCMap = _pNExpandedMap;
	for (i=0; i < _iExMapSize; i++) {
		for (j=0; j < _iExMapSize; j++) {
			for (k=0; k < _iExMapSize / 4; k++) {
				*pMap = 0;
				*pCMap = 0;
				pMap++;
				pCMap++;
			}
		}
	}
		
	_pMap = _pExpandedMap + __OFFSET_3D(1,1,1);
	_pNMap = _pNExpandedMap + __OFFSET_3D(1,1,1);
	pMap = _pMap;
	pCMap = _pCMap;
	pNCMap = _pNCMap;
	
	// read the initialize status from input file
	for (i = 0; i < _iMapSize; i++) {
		for (j = 0; j < _iMapSize; j++) {
			fgets (str, _iMapSize * 3, stdin);
					
			#ifdef __TESTINPUT
			printf ("Line :: %d %d :: \n%s \n", i, j, str);
			#endif
						
			for (str2 = strtok_r(str, " \n\0", &tok); *str2 != '\n'; str2 = strtok_r(NULL, " ", &tok)) {
				*pMap = (char)atoi (str2);				
				#ifdef __TESTINPUT
				printf("%d ",  *pMap);
				#endif				
				pMap++;
								
				*pCMap = 0;
				pCMap++;
				
				*pNCMap = 0;
				pNCMap++;
			}
			
			pMap += __OFFSET_Y;
			#ifdef __TESTINPUT
			printf("\n");
			#endif
		}
		pMap += __OFFSET_Z;
	}
		
	_ppMap = _pMap;
	_ppNMap = _pNMap;
	
	_ppCMap = _pCMap;
	_ppNCMap = _pNCMap;
		
	// Calculate which cell can be changed the status
	pCMap = _pCMap;
	for (i = 0; i < _iMapSize; i++) {
		for (j = 0; j < _iMapSize; j++) {
			for (k = 0; k < _iMapSize; k++) {
				
				/*
				// 초기 입력값에서 변화 여부를 계산				
				if (CalcDoA (k, j, i) == 1) {					
					// 변화가 필요한 값들만 따로 정리
					pCMap = _pCMap + ((i * _iMapSize + j) * _iMapSize) + k;
					
					*pCMap = 1;
				}
				*/
				*pCMap = 1;
				pCMap++;
				
			}			
		}
	}	
}


// Child Thread를 생성하고 관련 변수를 초기화 한다
void ThreadInit (void)
{
	int n, m, tid;
	int *arr;
	char * pChar;
	int * pInt;
	
	#ifdef __TESTMUTEX
	printf ("Initializing Threads\n");
	#endif
	
	// 맵 크기에 따른 Thread 개수 설정
	n = _iMapSize / __THREADDIV;
	_nThreads = n * n * n;
	if (_nThreads  == 0) {
		_nThreads = 1;
	}
	if (_nThreads > __THREADMAX) {
		_nThreads = __THREADMAX;
	}
	
	_pThread = malloc (sizeof (pthread_t) * (_nThreads - 1));
	
	// 검사 체크 맵 초기화
	_pMapCheck = malloc (sizeof (char) * _iMapSize);
	pChar = _pMapCheck;
	for (n=0; n < _iMapSize; n++) {
		*pChar = 0;
		pChar++;
	}
	
	// Thread별 변수 초기화
	_ThreadCnt = malloc (sizeof (int) * _nThreads);
	arr = malloc (sizeof (int) * _nThreads);
	pInt = _ThreadCnt;
	for (n = 0; n < _nThreads; n++) {
		arr[n] = n;
		*pInt = 0;
		pInt++;
	}
	
	// Child Thread 생성
	for (n = 1; n < _nThreads; n++) {
		tid = pthread_create(_pThread, NULL, ChildMain, &arr[n]);
		
		if (tid < 0) {
			printf ("Creating Thread Failed\n");
			exit (-1);
		}
	}
	
	// 변수 초기화
	_iReady = 0; // child들에게 한 차수를 시작할 준비를 알리는데 사용
	_iFinParent = 0; // child들에게 한 차수의 종합이 끝났음을 알리는데 사용
	_iEndSign = 0; // child들에게 모든 연산 종료를 알리는데 사용
	_iEndCnt = 0; // 한 차수에서 계산이 끝난 Thread의 갯수를 세는데 사용
	_CheckSize = _iMapSize;
	_cLoopCheck = 0; // 매 차수에서 Check Map에서 확인할 값
	
	// Mutex 초기화
	pthread_mutex_init (&_mEndCnt, NULL); // Initializing Mutex which synchronize the End count
	pthread_mutex_init (&_mMapCheck, NULL); // Initializing Mutex which synchronize the End count
}


// Child Thread의 Main Function
void *ChildMain (void * threadN)
{
	int ThreadN, i, StartArr, ChangeCnt;	
	int * CntBuffer;
	
	// 자신의 thread 번호 보관
	ThreadN = *(int *)threadN;
	
	#ifdef __TESTMUTEX
	printf ("Child Thread %d has been created by parents\n", *(int *) threadN);
	#endif
	
	CntBuffer = _ThreadCnt + *(int *)threadN;
	
	// lifegame 시작
	// Parent가 종료를 알릴때까지 계속 작동
	while ( _iEndSign != 1) {
		
		// Thread별로 변수 준비
		*CntBuffer = 0;
		ChangeCnt = 0;
		
		//i = StartArr;
		i = ThreadN;
		
		// parent가 준비를 알릴때까지 대기
		#ifdef __SPIN
		while (_iReady != 1) continue;
		#endif
				
		// 더 이상 계산이 없을 때까지 이번 차수 계산
		while (ChangeCnt >= 0) {
			*CntBuffer += ChangeCnt;
			ChangeCnt = Child_LifeGame (&i);			
			
			#ifdef __TESTMUTEX
			printf ("Thread [ %d ] :: Line [ %d ] Change counter [ %d ]\n", ThreadN, i, ChangeCnt);
			#endif
		}
		
		// 종료 카운트 증가
		pthread_mutex_lock (&_mEndCnt);
		_iEndCnt++;
		pthread_mutex_unlock (&_mEndCnt);
		
		#ifdef __TESTMUTEX
		printf ("Thread %d finish calculate [ %d ]\n", ThreadN, i);
		#endif
		
		// 종합이 완료될 때까지 대기
		#ifdef __SPIN
		while (_iFinParent != 1) ;
		#endif
		
		// 종료 카운트 감소하고 준비단계로 이동
		pthread_mutex_lock (&_mEndCnt);
		_iEndCnt--;
		pthread_mutex_unlock (&_mEndCnt);
	}
	
	pthread_exit (NULL);
}


// 입력된 차수만큼 Life Game을 실행한다
void LifeGame (void)
{
	int i, j, ThreadN, *pTmp;
	double time;
	struct timeval lt, ll;
	char * ppTmp;
	int ChangeCnt, SumChangeCnt;
	
	#ifdef __TESTBASIC
	printf ("Test BASIC Algorithm\n");
	#endif
	
	gettimeofday(&lt, NULL);
	
	for (i = 0; i < _iSteps; i++) {
		#ifdef __TESTBASIC
		printf ("%dth Loop\n", i);
		#endif
		
		// 한 세대를 돌기 전에 변수 준비
		_iEndCnt = 0;
		_iFinParent = 0;
		_cLoopCheck = _cLoopCheck ^ 0x01;
		SumChangeCnt = 0;
		ChangeCnt = 0;
		
		// 준비 완료 설정
		_iReady = 1;
		
		#ifdef __TESTMUTEX
		printf ("Parent thread calculate Start\n");
		#endif
		
		// 자신의 thread 번호 입력
		ThreadN = 0;
				
		// 더 이상 계산이 없을 때까지 이번 차수 계산
		while (ChangeCnt >= 0) {
			SumChangeCnt += ChangeCnt;
			ChangeCnt = Child_LifeGame (&ThreadN);
			
			#ifdef __TESTMUTEX
			printf ("Thread [ 0 ] :: Line [ %d ] Change counter [ %d ]\n", ThreadN, ChangeCnt);
			#endif
		};
		
		// 종료 카운트 증가
		pthread_mutex_lock (&_mEndCnt);
		_iEndCnt++;
		pthread_mutex_unlock (&_mEndCnt);
			
		#ifdef __TESTMUTEX
		printf ("Parent thread finish calculate\n");
		#endif
			
		// 모든 Thread 완료 대기
		#ifdef __SPIN
		while (_iEndCnt != _nThreads);
		#endif
				
		// 다음 세대 맵과 현재 세대 맵 교체
		ppTmp = _ppMap;
		_ppMap = _ppNMap;
		_ppNMap = ppTmp;
		
		ppTmp = _ppCMap;
		_ppCMap = _ppNCMap;
		_ppNCMap = ppTmp;
		
		// 모든 Thread의 Change Cnt 종합
		pTmp = _ThreadCnt;
		for (j=1; j < _nThreads; j++) {
			pTmp++;
			SumChangeCnt += *pTmp;
		}
		
		// 종료 완료 설정
		_iReady = 0;
		_iFinParent = 1;
		
		#ifdef __TESTCNT
		printf ("%d th Change Count :: %d\n", i+1, SumChangeCnt);
		#endif
		
		// 모든 child가 준비상태로 돌아갈때까지 대기
		#ifdef __SPIN
		while (_iEndCnt != 1); // Parent 제외한 모든 Child가 준비상태로 전환된 것 확인
		#endif
		
		if (SumChangeCnt == 0) {
			break;
		}
	}
	
	// Child에게 종료 알림
	_iEndSign = 1;
	
	gettimeofday(&ll, NULL);
	time = (double)(ll.tv_sec - lt.tv_sec) + (double)(ll.tv_usec - lt.tv_usec) / 1000000.0;
	fprintf (stderr, "Time : %.6lf\n", time);
	
}


// Child thread가 실행하는 life game
int Child_LifeGame (int * StartLine)
{
	char * pMap, * ppCMap, *ppNMap;
	int iZ, iY, i, iOffset;
	int ChangeCnt = 0;
	
	if (*StartLine >= _CheckSize) {
		return -1;
	}
	
	#ifdef __TESTMUTEX
	printf ("Searching at [ %d ] Total [ %d ] \n", *StartLine, _CheckSize);
	#endif
	
	// Check Map에서 비어있는 Line 선택
	pMap = _pMapCheck + *StartLine;		
	while (1) {
		if (*pMap != _cLoopCheck) {
			pthread_mutex_lock (&_mMapCheck);
			
			if (*pMap != _cLoopCheck) {
				*pMap = _cLoopCheck;
				pthread_mutex_unlock (&_mMapCheck);
				break;
			}
			
			pthread_mutex_unlock (&_mMapCheck);
		}
		
		pMap++;
		*StartLine =  *StartLine + 1;
		
		if (*StartLine >= _CheckSize) {
			return -1;
		}	
	}
	
#ifdef __TESTMUTEX
	printf ("Select [ %d ] th Line of Total [ %d ] Lines\n", *StartLine, _CheckSize);
#endif
	
	// 선택된 x,y plane의 Z 좌표 계산
	iZ = pMap - _pMapCheck;
		
	// Map 위치 계산
	iOffset = __OFFSET_1D(iZ);
	pMap = _ppMap + iOffset;
	ppNMap = _ppNMap + iOffset;
	ppCMap = _ppCMap + iZ * _iMapSize * _iMapSize;
		
	// 해당 Line에 있는 Cell들의 생존상태 계산
	for (iY=0; iY < _iMapSize; iY++) {
		for (i = 0; i < _iMapSize; i++) {
			if (*ppCMap == 1) {
				*ppCMap = 0; // 검사 완료 된 셀을 초기화
					
				if (CalcDoA (i, iY, iZ) == 1) {						
					MakeChange (i, iY, iZ); // 변했을 경우 주위 셀을 다음 Change map에 추가
					*ppNMap = *pMap ^ 0x01;	// 다음 상태에 반전으로 입력
					ChangeCnt++;
						
				} else {
					*ppNMap = *pMap;
				}
					
			} else {
				*ppNMap = *pMap;
			}
				
			pMap++;
			ppNMap++;
			ppCMap++;
		}
		pMap += __OFFSET_Y;
		ppNMap += __OFFSET_Y;
	}
		
	return ChangeCnt;	
}


// 파일 출력
void Out (void)
{
	FILE *ofp;
	int i, j, k;
	char * pMap;
	char cTmp;
	
	ofp = fopen ("output.life", "w");
	if (ofp == NULL) {
		printf ("Output File Open Failed\n");
		exit(-1);
	}
	
	fprintf (ofp, "%d %d %d %d %d %d %d\n", _iMapSize, _iD1, _iD2, _iL1, _iL2, _iSteps, __THREADMAX);
	
	pMap = _ppMap;
	
	for (i=0; i < _iMapSize; i++) {
		for (j=0; j < _iMapSize; j++) {
			for (k=0; k < _iMapSize; k++) {
				cTmp = (char)((int)*pMap + 0x30);
				fprintf(ofp, "%c ", cTmp);
				pMap++;
			}
			fprintf(ofp, "\n");
			pMap += __OFFSET_Y;
		}
		pMap += __OFFSET_Z;
	}
}


// dynamic allocation 된 memory 영역 해제
void Terminate (void) 
{
	free (_pExpandedMap);	
	free (_pNExpandedMap);
	free (_pNCMap);
	free (_pCMap);
		
	return;
}



// 한 세포의 생존상태를 검사한다
// return value가 1이면 생존상태가 변하고
// return value가 0이면 그대로 유지된다.
int CalcDoA (int x, int y, int z)
{
	char cell;
	char * pMap;
	void * pVoid;
	int nLiv = 0;
	int xStart, yStart, zStart, xRep, yRep, zRep;
	int i, j, k;
	int sum = 0;
	
	// 검사할 세포의 상태 로드
	pMap = _ppMap;
	pMap += __OFFSET_3D(x,y,z);
	cell = *pMap;			
	pMap += __OFFSET_3D(-1, -1, -1);
	
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	pMap += _iExMapSize * (_iExMapSize - 2);
	
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	pMap += _iExMapSize * (_iExMapSize - 2);
	
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	pMap += _iExMapSize;
	sum += *(int *)pMap;
	
	pVoid = &sum;
	nLiv = *(char *)pVoid;
	pVoid++;
	nLiv += *(char *)pVoid;
	pVoid++;
	nLiv += *(char *)pVoid;
		
	// 살아있는 세포 검사
	if (cell == 1) {
		nLiv--;	// 자기 자신은 카운트에서 제외
		// 사망 검사
		if ((nLiv < _iD1) || (nLiv > _iD2)) {
			return 1;
		} else {
			return 0;
		}
		
	// 죽은 세포 검사
	} else {
		// 생존 검사
		if ((nLiv > _iL1) && (nLiv < _iL2)) {
			return 1;
		} else {
			return 0;
		}
	}
}


// 세포 위치를 입력받아 세포 상태를 갱신하고 주위 셀과 함께 Change map에 입력하고 새로 추가해야 할 경우 List에도 등록한다
void MakeChange (int x, int y, int z)
{
	int ix, iy, iz;
	int xRep, yRep, zRep;
	int xStart, yStart, zStart;	
	char * pChar;
		
	// 상태 변화 맵에 자기 자신과 주위 세포에 변화 여부를 기록하고 변화하는 세포들을 리스트에 등록한다
	// 위치에 따른 검사 시작점과 반복 횟수 설정
	if (x == 0) {
		xRep = 2;
		xStart = x;
	} else if (x == _iMapSize-1){
		xRep = 2;
		xStart = x-1;
	} else {
		xRep = 3;
		xStart = x-1;
	}
	if (y == 0) {
		yRep = 2;
		yStart = y;
	} else if (y == _iMapSize-1) {
		yRep = 2;
		yStart = y-1;
	} else {
		yRep = 3;
		yStart = y-1;
	}
	if (z == 0) {
		zRep = 2;
		zStart = z;
	} else if (z == _iMapSize-1) {
		zRep = 2;
		zStart = z-1;
	} else {
		zRep = 3;
		zStart = z-1;
	}
		
	// 입력 시작 위치 설정
	pChar = _ppNCMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
	
	for (iz = 0; iz < zRep; iz++) {
		for (iy = 0; iy < yRep; iy++) {
			if (xRep == 3) {
				*pChar = 1;
				*(pChar+1) = 1;
				*(pChar+2) = 1;
				pChar += 3;
			} else {
				*pChar = 1;
				*(pChar+1) = 1;
				pChar += 2;
			}			
			
			pChar += _iMapSize - xRep;
		}
		pChar += _iMapSize * (_iMapSize - yRep);
	}
	
	return;
}

