
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/time.h>

//#define __TESTINPUT
//#define __TESTBASIC
#define __TESTPOOL

#define __BASIC
//#define __ADVANCED



////// Struct Declaration
struct pos {
	int x;
	int y;
	int z;
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

// Game of Life Function
int SearchMap (void);
int CalcDoA (int x, int y, int z);
int MakeChange (int x, int y, int z);

// List 관리
void ListInit (struct list * head, int * listcnt);
struct list * ListPop (struct list * head, int * listcnt);
void ListPush (struct list * head, struct list * node, int * listcnt);

// Node pool 관리
struct list * PoolGet (void);
void PoolFree (struct list * node);

// Change list, map 관리
void ChangeInsert (void);
void ChangeDelete (void);

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
char * _ppMap;
char * _ppNMap;
#endif

#ifdef __ADVANCED
// Changed Cell Map
char * _pCMap;
#endif

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
	
	return 0;
}


// 입력파일을 받아 Life Game을 초기화 시킨다
void init (void)
{
	int i, j, k;
	char *pMap, *pCMap;
	char *str, *str2, *tok;
	
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
	#endif
	#ifdef __ADVANCED
	_pCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	#endif
		
	pMap = _pMap;
	#ifdef __ADVANCED
	pCMap = _pCMap;
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
								
				#ifdef __ADVANCED
				*pCMap = 0;
				pCMap++;
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
	#endif
	
	#ifdef __ADVANCED
	// List Initialize
	ListInit (&_lPool, &_iPoolCnt);
	ListInit (&_lChange, &iChangeCnt);
	
	// Calculate which cell can be changed the status
	for (i = 0; i < _iMapSize; i++) {
		for (j = 0; j < _iMapSize; j++) {
			for (k = 0; k < _iMapSize; k++) {
				// 좌표에서 변화 가능 여부를 계산해서 Change List에 입력
				if (CalcDoA (k, j, i) == 1) {
					ChangeInsert (k, j, i);
				}
			}
		}
	}
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
		
		SearchMap();
		ppTmp = _ppMap;
		_ppMap = _ppNMap;
		_ppNMap = _ppMap;
		#endif
		
		#ifdef __ADVANCED
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


// 변경 리스트를 따라 한 차수의 맵을 갱신한다
// 이번에 변경 된 cell의 갯수를 반환한다
int SearchMap (void)
{
#ifdef __BASIC
	int i, j, k;
	char * ppMap, * ppNMap;
	
	ppMap = _ppMap;
	ppNMap = _ppNMap;
	
	for (i=0; i < _iMapSize; i++) {
		for (j=0; j < _iMapSize; j++) {
			for (k=0; k < _iMapSize; k++) {
				if (CalcDoA (k, j, i) == 1) {
					*ppNMap = *ppMap ^ 0x01;
				} else {
					*ppNMap = *ppMap;
				}
				ppMap++;
				ppNMap++;
			}
		}
	}
	
	return 0;
#endif

#ifdef __ADVANCED	
	int ChangeCnt = _iChangeCnt;
	int i;
	// 차수 시작할 때 Change List에 있는 node에 대해서만 변화 검사
	for (i = 0; i < ChangeCnt; i++) {
		// 변경 List에서 Node를 Pop 한다
	
		// 상태 변화 맵에서 bit를 0으로 바꾸고 상태 변화를 검사한다
	
		// 변화된 세포는 리스트 뒤에 추가해서 재검사를 막는다
	}
	
	// 변화된 세포들에 대해 맵을 갱신하고 리스트에 추가적으로 등록한다
	for (i = 0; i < _iChangeCnt; i++) {
		// 
	}
#endif
}


// 한 세포의 생존상태를 검사한다
// return value가 1이면 생존상태가 변하고
// return value가 0이면 그대로 유지된다.
int CalcDoA (int x, int y, int z)
{
	char cell;
	char * pMap = _pMap;
	int nLiv = 0;
	int xStart, yStart, zStart, xRep, yRep, zRep;
	int i, j, k;
	
	// 검사할 세포의 상태 로드
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
	pMap = _pMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
	
	// 살아있는 세포 갯수 체크
	for (i = 0 ; i < zRep; i++) {
		for (j = 0; j < yRep; j++) {
			for (k = 0; k < xRep; k++) {
				nLiv += (int)(*pMap);
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


// 세포 위치를 입력받아 상태를 반전시키고 상태 변화 리스트와 맵에 주위 셀과 함께 등록한다
int MakeChange (int x, int y, int z)
{
	// 상태를 반전시킨다
	
	// 상태 변화 맵에 자기 자신과 주위 세포에 변화 여부를 기록하고 변화하는 세포들을 리스트에 등록한다
	
}


// 전달 받은 head에서 가장 처음 있는 node를 pop한다
// List에 node가 없을 경우 NULL을 반환한다
struct list * ListPop (struct list * head, int * listcnt)
{
	struct list * pTmp;
	
	if (*listcnt == 0) {
		return NULL;
	}
	
	pTmp = head->next;	
	// list에서 삭제
	pTmp->next->prev = head;
	head->next = pTmp->next;
	(*listcnt)--;
	
	return pTmp;
}


// 전달 받은 head의 맨 뒤에 전달 받은 node를 push 한다
void ListPush (struct list * head, struct list * node, int * listcnt)
{
	struct list * pTmp;
	
	// List에 삽입
	pTmp->next = head->next;
	pTmp->prev = head;
	head->next->prev = pTmp;
	head->next = pTmp;	
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

void ChangeInsert (void)
{
}

void ChangeDelete (void)
{
}

