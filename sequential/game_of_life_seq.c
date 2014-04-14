
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/time.h>

//#define __TESTINPUT

#define __BASIC
//#define __TESTBASIC
//#define __NOOUTPUT


//===================================
// macro function
//===================================
#define __OFFSET_3D(x, y, z)	((z * _iExMapSize + y) * _iExMapSize + x)
#define __OFFSET_2D(y,z) 		((z * _iExMapSize + y) * _iExMapSize)
#define __OFFSET_1D(z)		(z * _iExMapSize * _iExMapSize)
//===================================


//===================================
// Function Declaration
//===================================
void init (void);
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
int __THREADMAX = 0;

// Map
int _iExMapSize;
char * _pExpendedMap;
char * _pMap;

char * _pAMap;
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
	str = malloc (sizeof (char) * _iMapSize * 3);
	_pMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	_pAMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	_pCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);
	_pNCMap = malloc (sizeof (char) * _iMapSize * _iMapSize * _iMapSize);	
		
	pMap = _pMap;
	pCMap = _pCMap;
	pNCMap = _pNCMap;
	
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
	
	_ppMap = _pMap;
	_ppNMap = _pAMap;
	
	_ppCMap = _pCMap;
	_ppNCMap = _pNCMap;
		
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
		
}


// 입력된 차수만큼 Life Game을 실행한다
void LifeGame (void)
{
	int i;
	double time;
	struct timeval lt, ll;		
	char * ppTmp;
	int ChangeCnt;
	
	#ifdef __TESTBASIC
	printf ("Test BASIC Algorithm\n");
	#endif
	
	gettimeofday(&lt, NULL);
	
	for (i = 0; i < _iSteps; i++) {		
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
		}
	}
}


// dynamic allocation 된 memory 영역 해제
void Terminate (void) 
{

	free (_pMap);	
	free (_pAMap);
	free (_pNCMap);
	free (_pCMap);
		
	return;
}

// 변경 리스트를 따라 한 차수의 맵을 갱신한다
// 다음에 검사해야 할 세포의 갯수를 반환한다
int SearchMap (void)
{
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
	pMap = _ppMap;

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
	pMap = _ppMap + ((zStart * _iMapSize + yStart) * _iMapSize) + xStart;
	
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



