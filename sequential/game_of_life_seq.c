
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/time.h>

//#define __TESTINPUT
//#define __TESTPOOL

#define __BASIC
//#define __TESTBASIC
//#define __NOOUTPUT

//#define __ADVANCE
//#define __TESTADV


////// Struct Declaration
struct pos {
	short x;
	short y;
	short z;
};

struct list {
	struct list * next;
	struct list * prev;
	struct pos p;
};

////// Struct End



////// Function Declaration
void init (void);
void LifeGame(void);
void Out (void);

void Terminate (void);

// Game of Life Function
int SearchMap (void);
int CalcDoA (int x, int y, int z);
void MakeChange (int x, int y, int z);

// List 관리
void ListInit (struct list * head, int * listcnt);
struct list * ListPop (struct list * head, int * listcnt);
void ListPush (struct list * head, struct list * node, int * listcnt);

// Node pool 관리
struct list * PoolGet (void);
void PoolFree (struct list * node);

// Change list, map 관리
void ChangeInsert (int x, int y, int z, char * CMapPos);
struct list * ChangePop (void);

////// Function Declaration End



////// Global Variable
// Initialize Value

int _iMapSize = 0;
int _iD1 = 0;
int _iD2 = 0;
int _iL1 = 0;
int _iL2 = 0;
int _iSteps = 0;

// Map
char * _pMap;

#ifdef __BASIC
char * _pAMap;
char * _pNCMap;

char * _ppMap;
char * _ppNMap;
char * _ppCMap;
char * _ppNCMap;
#endif

// Changed Cell Map
char * _pCMap;

// list of the Cell need to change
struct list _lChange;
int _iChangeCnt = 0;

// Memory Pool
struct list _lPool;
int _iPoolCnt = 0;

// Test Value
#ifdef __TESTPOOL
int _iPoolEmptyCnt = 0;
#endif

////// Global Variable End


////// Function Start
int main (void)
{
	init ();
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

#ifdef __BASIC
	char *pNCMap;
#endif
	
	
	str = malloc (100);
	
	fgets(str, 100, stdin);
	
#ifdef __TESTINPUT
	printf("input file :: %s\n", str);
#endif
	
	str2 = strtok_r (str, " ", &tok);
	_iMapSize = atoi (str2);
	
	str2 = strtok_r (NULL, " ", &tok);
	_iD1 = atoi (str2);
	
	str2 = strtok_r (NULL, " ", &tok);
	_iD2 = atoi (str2);
	
	str2 = strtok_r (NULL, " ", &tok);
	_iL1 = atoi (str2);
	
	str2 = strtok_r (NULL, " ", &tok);
	_iL2 = atoi (str2);
	
	str2 = strtok_r (NULL, " ", &tok);
	_iSteps = atoi (str2);

	free (str);

#ifdef __TESTINPUT
	printf ("%d %d %d %d %d %d\n", _iMapSize, _iD1, _iD2, _iL1, _iL2, _iSteps);
#endif
	
	// Create Map
	str = malloc (sizeof (char) * _iMapSize * 3);
	_pMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	#ifdef __BASIC
	_pAMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	_pNCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	#endif
	
	_pCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
		
	pMap = _pMap;
	pCMap = _pCMap;
#ifdef __BASIC
	pNCMap = _pNCMap;
#endif
	
	for (i = 0; i < _iMapSize; i++) {
		for (j = 0; j < _iMapSize; j++) {
			fgets (str, _iMapSize * 3, stdin);
					
			#ifdef __TESTINPUT
			printf ("Line :: %d %d :: \n%s \n", i, j, str);
			#endif
						
			for (str2 = strtok_r(str, " ", &tok); *str2 != '\n'; str2 = strtok_r(NULL, " ", &tok)) {
				*pMap = (char)atoi (str2);
				
				#ifdef __TESTINPUT
				printf("%d ",  *pMap);
				#endif
				
				pMap++;
								
				*pCMap = 0;
				pCMap++;
				
				#ifdef __BASIC
				*pNCMap = 0;
				pNCMap++;
				#endif
			}
						
			#ifdef __TESTINPUT
			printf("\n");
			#endif
		}
	}
	
	#ifdef __BASIC
	_ppMap = _pMap;
	_ppNMap = _pAMap;
	
	_ppCMap = _pCMap;
	_ppNCMap = _pNCMap;
	#endif
	
	#ifdef __ADVANCE
	// List Initialize
	ListInit (&_lPool, &_iPoolCnt);
	ListInit (&_lChange, &_iChangeCnt);
	#endif
	
	// Calculate which cell can be changed the status
	for (i = 0; i < _iMapSize; i++) {
		for (j = 0; j < _iMapSize; j++) {
			for (k = 0; k < _iMapSize; k++) {
				
				// 초기 입력값에서 변화 여부를 계산
				if (CalcDoA (k, j, i) == 1) {
					
					// 변화가 있는 값들만 따로 정리
					pCMap = _pCMap + ((i * _iMapSize + j) * _iMapSize) + k;
					
					#ifdef __BASIC
					*pCMap = 1;
					#endif
					
					#ifdef __ADVANCE
					ChangeInsert (k, j, i, pCMap);
					#endif
				}
				
			}
		}
	}
	
	
	#ifdef __TESTADV
	printf ("Initial Change List [ %d ]\n", _iChangeCnt);
	#endif
}


// 입력된 차수만큼 Life Game을 실행한다
void LifeGame (void)
{
	int i;
	double time;
	struct timeval lt, ll;
		
	#ifdef __BASIC
	char * ppTmp;
	int ChangeCnt;
	#endif
	
	#ifdef __TESTBASIC
	printf ("Test BASIC Algorithm\n");
	#endif
	
	gettimeofday(&lt, NULL);
	
	for (i = 0; i < _iSteps; i++) {
		#ifdef __BASIC
		
		#ifdef __TESTBASIC
		printf ("%dth Loop\n", i);
		#endif
		
		ChangeCnt = SearchMap();
		
		ppTmp = _ppMap;
		_ppMap = _ppNMap;
		_ppNMap = ppTmp;
		
		ppTmp = _ppCMap;
		_ppCMap = _ppNCMap;
		_ppNCMap = ppTmp;
		
		if (ChangeCnt == 0) {
			break;
		}
		#endif
		
		#ifdef __ADVANCE
		// 한 차수만큼 진행시키고 변화된 세포의 갯수를 검사한다
		if (SearchMap () == 0) {
			// 아무런 세포도 바뀌지 않았을 경우 바로 종료한다
			break;
		}
		#endif
	}
	
	gettimeofday(&ll, NULL);
	time = (double)(ll.tv_sec - lt.tv_sec) + (double)(ll.tv_usec - lt.tv_usec) / 1000000.0;
	fprintf (stderr, "Time : %.6lf\n", time);
	
}


// 파일 출력
void Out (void)
{
	FILE *ofp;
	int i, j, k;
	char * pMap;
	char cTmp;
	
	ofp = fopen ("input.sol", "w");
	if (ofp == NULL) {
		printf ("Output File Open Failed\n");
		exit(-1);
	}
	
#ifdef __BASIC
	pMap = _ppMap;
#endif
#ifdef __ADVANCE
	pMap = _pMap;
#endif	
	
	for (i=0; i < _iMapSize; i++) {
		for (j=0; j < _iMapSize; j++) {
			for (k=0; k < _iMapSize; k++) {
				cTmp = (char)((int)*pMap + 0x30);
				fprintf(ofp, "%c ", cTmp);
				pMap++;
			}
			fprintf(ofp, "\n");
		}
	}
}


// dynamic allocation 된 memory 영역 해제
void Terminate (void) 
{
#ifdef __ADVANCE
	struct list * pList;
	int i;
#endif

	free (_pMap);	
	#ifdef __BASIC
	free (_pAMap);
	free (_pNCMap);
	#endif	
	free (_pCMap);
	
#ifdef __ADVANCE
	while (_iPoolCnt != 0) {
		pList = PoolGet ();
		
		free (pList);
	}
#endif
	
	return;
}

// 변경 리스트를 따라 한 차수의 맵을 갱신한다
// 다음에 검사해야 할 세포의 갯수를 반환한다
int SearchMap (void)
{
#ifdef __BASIC
	int i, j, k, ChangeCnt = 0;
	char * ppMap, * ppNMap, *ppCMap, *ppNCMap;
	
	ppMap = _ppMap;
	ppNMap = _ppNMap;
	ppCMap = _ppCMap;
	ppNCMap = _ppNCMap;
	
	for (i=0; i < _iMapSize; i++) {
		for (j=0; j < _iMapSize; j++) {
			for (k=0; k < _iMapSize; k++) {
				
				if (*ppCMap == 1) {
					*ppCMap = 0; // 검사 완료 된 셀을 초기화
					
					if (CalcDoA (k, j, i) == 1) {						
						MakeChange (k, j, i); // 변했을 경우 주위 셀을 다음 Change map에 추가
						*ppNMap = *ppMap ^ 0x01;	// 다음 상태 반전으로 입력
						ChangeCnt++;
						
					} else {
						*ppNMap = *ppMap;
					}
					
				} else {
					*ppNMap = *ppMap;
				}
				
				ppMap++;
				ppNMap++;
				ppCMap++;
			}
		}
	}
	
	#ifdef __TESTBASIC
	printf ("Change Cnt [ %d ]\n", ChangeCnt);
	#endif
	
	return ChangeCnt;
#endif

#ifdef __ADVANCE	
	int ChangeCnt = _iChangeCnt;
	int i;
	char * pChar;
	struct list * pList;
	
	#ifdef __TESTADV
	printf ("Start Change List [ %d ]\n", _iChangeCnt);
	#endif
	
	// 차수 시작할 때 Change List에 있는 node에 대해서만 변화 검사 (중복 검사 방지)
	for (i = 0; i < ChangeCnt; i++) {
		// 변경 List에서 Node를 Pop 한다
		pList = ChangePop ();
	
		// 상태 변화를 검사한다
		if (CalcDoA (pList->p.x, pList->p.y, pList->p.z) == 1) {
			// 변화된 세포는 변화 리스트 뒤에 다시 추가한다
			ListPush (&_lChange, pList, &_iChangeCnt);
			
		} else {
			// 변화가 없을 경우 node 반환
			PoolFree (pList);
		}
	}
	
	#ifdef __TESTADV
	printf ("Changed Cell [ %d ]\n", _iChangeCnt);
	#endif
	
	// 변화된 세포들에 대해 맵을 갱신하고 리스트에 추가적으로 등록한다
	ChangeCnt = _iChangeCnt;
	for (i = 0; i < ChangeCnt; i++) {
		pList = ListPop (&_lChange, &_iChangeCnt);
		
		MakeChange (pList->p.x, pList->p.y, pList->p.z);
		
		PoolFree (pList);
	}
	
	#ifdef __TESTADV
	printf ("Next Change List [ %d ]\n", _iChangeCnt);
	printf ("Pool Cnt [ %d ]\n", _iPoolCnt);
	#endif
	
	return _iChangeCnt;
#endif
}


// 한 세포의 생존상태를 검사한다
// return value가 1이면 생존상태가 변하고
// return value가 0이면 그대로 유지된다.
int CalcDoA (int x, int y, int z)
{
	char cell;
	char * pMap;
	int nLiv = 0;
	int xStart, yStart, zStart, xRep, yRep, zRep;
	int i, j, k;
	
	// 검사할 세포의 상태 로드
#ifdef __BASIC
	pMap = _ppMap;
#endif
#ifdef __ADVANCE
	pMap = _pMap;
#endif
	pMap += (z * _iMapSize * _iMapSize) + (y * _iMapSize) + x;
	cell = *pMap;
	
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
		
	// 검사 시작 위치 설정
#ifdef __BASIC
	pMap = _ppMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
#endif
#ifdef __ADVANCE
	pMap = _pMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
#endif
	
	// 살아있는 세포 갯수 체크
	for (i = 0 ; i < zRep; i++) {
		if (yRep == 3) {
			if (xRep == 3) {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			} else {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			}						
			pMap += _iMapSize - xRep;
			
			if (xRep == 3) {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			} else {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			}						
			pMap += _iMapSize - xRep;
			
			if (xRep == 3) {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			} else {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			}						
			pMap += _iMapSize - xRep;
		} else {
			if (xRep == 3) {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			} else {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			}						
			pMap += _iMapSize - xRep;
			
			if (xRep == 3) {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			} else {
				nLiv += *pMap;
				pMap++;
				nLiv += *pMap;
				pMap++;
			}						
			pMap += _iMapSize - xRep;
		}			
		pMap += _iMapSize * (_iMapSize - yRep);
	}
	
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
	
#ifdef __ADVANCE
	// 세포 상태 갱신
	pChar = _pMap + ((z * _iMapSize + y) * _iMapSize) + x;
	*pChar = *pChar ^ 0x01;
#endif
		
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
#ifdef __BASIC
	pChar = _ppNCMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
#endif
#ifdef __ADVANCE
	pChar = _pCMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
#endif
	
	for (iz = 0; iz < zRep; iz++) {
		for (iy = 0; iy < yRep; iy++) {
			for (ix = 0; ix < xRep; ix++) {
				
				#ifdef __BASIC
				*pChar = 1;
				#endif
				#ifdef __ADVANCE
				ChangeInsert(xStart + ix, yStart + iy, zStart + iz, pChar);
				#endif
				
				pChar++;
			}
			pChar += _iMapSize - xRep;
		}
		pChar += _iMapSize * (_iMapSize - yRep);
	}
	
	return;
}


// 전달 받은 head에서 가장 처음 있는 node를 pop한다
// List에 node가 없을 경우 NULL을 반환한다
struct list * ListPop (struct list * head, int * listcnt)
{
	struct list * pTmp;
	
	if (*listcnt == 0) {
		return NULL;
	}
	
	// list에서 pop
	pTmp = head->next;	
	pTmp->next->prev = head;
	head->next = pTmp->next;
	(*listcnt)--;
	
	return pTmp;
}


// 전달 받은 head의 맨 뒤에 전달 받은 node를 push 한다
void ListPush (struct list * head, struct list * node, int * listcnt)
{	
	// List의 맨 뒤에 삽입
	node->next = head;
	node->prev = head->prev;
	head->prev->next = node;
	head->prev = node;
	(*listcnt)++;
	
	return;
}


// List를 초기화 한다
void ListInit (struct list * head, int * listcnt)
{
	head->next = head;
	head->prev = head;	
	*listcnt = 0;
}

// Free된 좌표 structure가 저장 된 pool에서 메모리를 가져오고 pool이 비었을 경우 새로 할당한다
struct list * PoolGet (void)
{
	struct list * pTmp;
	
	// pool이 비었을 경우 새로 node를 할당
	if (_iPoolCnt == 0) {
		#ifdef __TESTPOOL
		_iPoolEmptyCnt++;
		#endif
		pTmp = malloc (sizeof (struct list));
		
		return pTmp;
		
	// Pool에 node가 있을 경우 Pool에서 node를 반출
	} else {
		pTmp = ListPop (&_lPool, &_iPoolCnt);
		
		return pTmp;
	}
}

// 좌표 structure의 사용이 끝났을 경우 pool에 반환한다.
void PoolFree (struct list * node)
{
	// Pool List에 Node를 입력
	ListPush (&_lPool, node, &_iPoolCnt);
}


// Change Map에 1로 표시하고 중복되지 않을 경우 Change List에 Insert 한다
// x, y, z 좌표와 Change Map에서 위치를 입력받는다
void ChangeInsert (int x, int y, int z, char * CMapPos)
{
	struct list *pTmp;
	
	// Change Map에서 0일 경우에만
	// 1로 변경하고 Change List에 Push
	if (*CMapPos == 0) {
		*CMapPos = 1;
		
		pTmp = PoolGet ();
		pTmp->p.x = x;
		pTmp->p.y = y;
		pTmp->p.z = z;
		ListPush (&_lChange, pTmp, &_iChangeCnt); 
	}
	
	return;
}


// Change Map에서 1로 표기된 것을 0으로 바꾸고 Change List에서 반환한다
// Change List가 비었을 경우 NULL을 반환한다
struct list * ChangePop (void)
{
	char * pChar;
	struct list * pTmp;
	int x, y, z;
	
	// Change List에서 node를 pop
	pTmp = ListPop (&_lChange, &_iChangeCnt);
	
	if (pTmp != NULL) {
		// 해당 위치의 Change Map을 0으로 변경
		x = pTmp->p.x;
		y = pTmp->p.y;
		z = pTmp->p.z;
		
		pChar = _pCMap + ((z * _iMapSize + y) * _iMapSize) + x;
		*pChar = 0;
	}
	
	return pTmp;
}


