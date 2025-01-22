/*=================================================
TF-partition
=================================================*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>

short	E=0;

double Weibull(double x, double a, double b){
    double  y;
    y = 1.0 - exp( - (pow( (x-0.5)/a, b) ) );
    return y;
}

int main(void){

	char	Fname_OP[64], Fname[64];

	char	buff[4096];
	FILE	*fpSrc, *fpDst;

	char	dataC[32];
	short	chrFlg;

	char	flag_SP;

	char	DMY[11], ID[9];
	int		cnt_OP[1440], cnt_TF, com;
	double	P_OP[1440], P_TF;
	double	Dmax_OP[1440], D50_OP[1440], Dmax_TF;
	double	TF[100], OP[1440][100];
	double	SP[100], FR[100], DR[100];
	double	SPmin;
	double	cSP[16];

	int		line, lineMax;

	double	tm_pre, tm;
	double	SP_d[7], FR_d[7], DR_d[7];
	double	dth[7] = {0.0, 0.1, 0.25, 0.5, 0.75, 0.9, 1.0};
	double	cls[100] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 
						2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 4.0,
						4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.0,
						6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 7.0, 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 7.8, 7.9, 8.0,
						8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 8.8, 8.9, 9.0, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.9, 10.0};
	double	a, b;

	double	pp, pp_1, wei_a, wei_b;
	double	cumOP, cumTF, cumSP, cumFR, cumDR;
	double	fTF, fSP, fFR, fDR;
	int		i, j, k, w1, w2;

	double	diff, diff_min, dummy;
	int		SP_limit;

	HANDLE hFind, hFind2;
	WIN32_FIND_DATA fd, fd2;

	//━━━━━━━━━━━━━━━━━━━━
	// First message
	//━━━━━━━━━━━━━━━━━━━━
	printf( "\n=========\n");
	printf( " TF partitioning\n");
    printf( "=========\n");

	//━━━━━━━━━━━━━━━━━━━━
	// Search RD2 data
	//━━━━━━━━━━━━━━━━━━━━
	hFind = FindFirstFile("RD2_*OP*.csv", &fd);

	if(hFind == INVALID_HANDLE_VALUE) {
		printf("\nThere is no data named [RD2*OP] in the directory.\n");
		if(E == 0){
			printf(" To terminate the program, key in something and 'Enter'.\n");
			scanf("%d", E);
			E = 0;
		}
		return 0;
	}

	do {

		Fname_OP[64] = '\0';

		char target_fn_first[16], target_fn_last[10];
		target_fn_first[0] = '\0';
		target_fn_last[0] = '\0';

		strncpy(target_fn_first, fd.cFileName, 15);
		target_fn_first[15] = '\0';
		strcpy(target_fn_last, fd.cFileName + strlen(fd.cFileName) - 9);

		sprintf(Fname_OP, "%s*%s", target_fn_first, target_fn_last);
		Fname_OP[strlen(Fname_OP)] = '\0';

		printf("\n======\n");
		printf("  Target OP: %s \n", fd.cFileName);
		printf("   look for: %s \n\n", Fname_OP);

		//━━━━━━━━━━━━━━━━━━━━━━
		// Store OP data
		//	cnt, Rain(mm), Dmax, D50, R-0.1/10.0
		//━━━━━━━━━━━━━━━━━━━━━━
		fpSrc = fopen( fd.cFileName, "r" );

		for(i=0; i<2; ++i)
			fgets(buff, 4096, fpSrc);	// Ignore the first 2 lines

		line = 0;
		while(NULL != fgets(buff, 4096, fpSrc)){

			chrFlg = 0;
			com = 0;
			i = j = 0;
			for(k=0; k<31; ++k) dataC[k] = '\0';

			if(line == 0){
				strncpy(DMY, buff, 10);
				DMY[10] = '\0';
			}

			// Read one character
			while('\0' != buff[i]){

				// When the comma immediately before the required data arrives, flag it and increment dataC by one character
				if(com==2 || com==3 || com==10 || com==13 || com >= 114 ){
					chrFlg = 1;
					dataC[j] = buff[i];

					++j;
				}

				if(buff[i] == ','){

					if(chrFlg == 1){
						chrFlg = 0;

						if(com ==  2)	cnt_OP[line] = atoi(dataC);
						if(com ==  3)	P_OP[line] = atof(dataC);
						if(com == 10){
							if(dataC[0] == 'N')   D50_OP[line] = 0.0;
							else  D50_OP[line] = atof(dataC);
						}
						if(com == 13){
							if(dataC[0] == 'N')   Dmax_OP[line] = 0.0;
							else  Dmax_OP[line] = atof(dataC);
						}
						if(com >= 114)	OP[line][com-114] = atof(dataC);
						j=0;
					}

					++com;
					for(k=0; k<32; ++k) dataC[k] = '\0';

				}
				++i;

			}
			++line;

		}
		lineMax = line;

		fclose(fpSrc);

		//━━━━━━━━━━━━━━━━━━━━━━
		// TF partition
		//━━━━━━━━━━━━━━━━━━━━━━
		// Search TF data
		hFind2 = FindFirstFile(Fname_OP, &fd2);
		if(hFind2 == INVALID_HANDLE_VALUE) {
			printf("There is no data named %s in the directory.\n", Fname_OP);
			if(E == 0){
				printf(" To terminate the program, key in something and 'Enter'.\n");
				scanf("%d", E);
				E = 0;

			}
			return 0;
		}

		do {

			if(NULL != strstr(fd2.cFileName, "OP"))   continue;

			sscanf(fd2.cFileName, "RD2_Aggr01min%s", &buff);
			printf("   Partitioning: %s ---\n  ", fd2.cFileName);

			fpSrc = fopen( fd2.cFileName, "r" );
			sprintf(Fname, "RD5_TFpart01min%s", buff);
			fpDst = fopen( Fname, "w");

			fgets(buff, 4096, fpSrc);
			fprintf(fpDst, "%s", buff);

			fgets(buff, 4096, fpSrc);

			fprintf(fpDst, "DMY,ID,P_OP,P_TF,cnt_OP,cnt_TF,D50_OP,Dmax_OP,fTF,f_direct,Weibull_a,Weibull_b,SP_limit,");
			fprintf(fpDst, "fSP,fFR,fDR,R_SP,R_FR,R_DR");
			fprintf(fpDst, ",SP_Dmin,SP_D10,SP_D25,SP_D50,SP_D75,SP_D90,SP_Dmax");
			fprintf(fpDst, ",FR_Dmin,FR_D10,FR_D25,FR_D50,FR_D75,FR_D90,FR_Dmax");
			fprintf(fpDst, ",DR_Dmin,DR_D10,DR_D25,DR_D50,DR_D75,DR_D90,DR_Dmax");
			for(j=0; j<100; ++j)	fprintf(fpDst, ",SP_%3.1f", cls[j]);
			for(j=0; j<100; ++j)	fprintf(fpDst, ",FR_%3.1f", cls[j]);
			for(j=0; j<100; ++j)	fprintf(fpDst, ",DR_%3.1f", cls[j]);
			fprintf(fpDst, "\n");

			for(line=0; line<lineMax; ++line){

				if((line+1)%10 == 0)   printf("  %04d", (line+1));
				if((line+1)%120 == 0)   printf("\n  ");

				fgets(buff, 4096, fpSrc);

				// Read one character
				chrFlg = com = w1 = 0;
				i = j = 0;
				while('\0' != buff[i]){

					// When the comma immediately before the required data arrives, flag it and increment dataC by one character
					if(com==1 || com==2 || com==3 || com==13 || com >= 114){
						chrFlg = 1;
						dataC[j] = buff[i];

						++j;
					}

					// Increase com every time a comma comes up
					if(buff[i] == ','){
						if(chrFlg == 1){
							chrFlg = 0;

							if(com==1){
								strncpy(ID, dataC, 8);
								ID[8] = '\0';
							}
							if(com== 2)	cnt_TF = atoi(dataC);
							if(com== 3)	P_TF = atof(dataC);
							if(com==13){
								if(dataC[0] == 'N')  Dmax_TF = 0.0;
								else  Dmax_TF = atof(dataC);
							}
							if(com >= 114){
								TF[com-114] = atof(dataC);
							}
							

							++w1;
							for(k=0; k<32; ++k) dataC[k] = '\0';
							j=0;
						}
						++com;
					}
					++i;

				}

				// TF data processing
				// In case of TF=0
				if(cnt_TF == 0){
					fTF = 0.0;

					fprintf(fpDst, "%s,%s,%f,%f,%d,%d,%f,%f,%f,NA,NA,NA,NA,NA,NA,NA,0,0,0", DMY, ID, P_OP[line], P_TF, cnt_OP[line], cnt_TF, D50_OP[line], Dmax_OP[line], fTF);
					for(j=0; j<7; ++j)	fprintf(fpDst, ",NA");
					for(j=0; j<7; ++j)	fprintf(fpDst, ",NA");
					for(j=0; j<7; ++j)	fprintf(fpDst, ",NA");
					for(j=0; j<100; ++j)	fprintf(fpDst, ",0");
					for(j=0; j<100; ++j)	fprintf(fpDst, ",0");
					for(j=0; j<100; ++j)	fprintf(fpDst, ",0");
					fprintf(fpDst, "\n");

					continue;
				}

				// In case of TF!=0
				// Determination of direct rain rate pp (calculated to 7 decimal places)
				flag_SP = 0;
				pp = pp_1 = 0.0;

				for(k=0; k<7; ++k){
					for(i=0; i<10; ++i){
						pp = pp_1 + pow(0.1, k) * (i+1) ;
						for(j=0; j<30; ++j){
							SP[j] = 0.0;

							if(TF[j] != 0.0){
								SP[j] = TF[j] - OP[line][j] * pp;
								if(SP[j] < 0.0){
									flag_SP = 1;
									break;
								}
							}
						}

						if(flag_SP == 1){
							pp_1 = pp - pow(0.1, k);
							flag_SP = 0;
							break;
						}
					}
				}
				if(k==7){
					pp -= 0.000001;
				}
				if(pp > 1.0){
					pp = 1.0;
				}

				// SP_limit
				k = 19;

				// Create a cumulative distribution of Release-TF
				SP[0] = cSP[0] = 0.0;
				for(j=1; j<=k; ++j){
					SP[j] = 0.0;
					if(TF[j] != 0.0){
						SP[j] = TF[j] - OP[line][j] * pp;
					}
					cSP[j] = cSP[j-1] + SP[j];
				}
				cumSP = cSP[k];
				for(j=0; j<=k; ++j){
					cSP[j] /= cumSP;
				}

				// Fitting of Weibull function
				diff_min = 9999.9;
				for(w1=0; w1<120; ++w1){
					a = 0.01*(w1+1);
					for(w2=0; w2<900; ++w2){
						b = 0.01*(w2+1);
						diff = 0.0;
						for(i=8; i<=k; ++i){
							diff += pow( Weibull(i*0.1+0.1, a, b) - cSP[i], 2);
						}
						if(diff < diff_min){
							diff_min = diff;
							wei_a = a;
							wei_b = b;
							SP_limit = k;
							dummy = cumSP;
						}
					}
				}

				for(int w1a=0; w1a<20; ++w1a){
					a = wei_a - 0.01 + (w1a+1) * 0.001;
					for(int w2a=0; w2a<20; ++w2a){
						b = wei_b - 0.01 + (w2a+1) * 0.001;
						diff = 0.0;
						for(i=9; i<=k; ++i){
							diff += pow( Weibull(i*0.1+0.1, a, b) - cSP[i], 2);
						}
						if(diff < diff_min){
							diff_min = diff;
							wei_a = a;
							wei_b = b;
							SP_limit = k;
							dummy = cumSP;
						}
					}
				}

				// Calculation of each component
				for(i=0; i<100; ++i){
					FR[i] = OP[line][i] * pp;
					if(TF[i] < FR[i])	FR[i] = TF[i];

					if(i<11){
						SP[i] = TF[i] - FR[i];
						DR[i] = 0.0;
					
					} else if(i<=SP_limit){
						SP[i] = ( Weibull(i*0.1+0.1, wei_a, wei_b) - Weibull(i*0.1, wei_a, wei_b) )*dummy;
						if(SP[i] > (TF[i] - FR[i]) )	SP[i] = TF[i] - FR[i];
						
						DR[i] = TF[i] - FR[i] - SP[i];
					
					} else {
						SP[i] = 0.0;
						DR[i] = TF[i] - FR[i];
					}
				}

				cumOP = cumTF = cumSP = cumFR = cumDR = 0.0;
				for(i=0; i<100; ++i){
					cumOP += OP[line][i];
					cumTF += TF[i];
					cumSP += SP[i];
					cumFR += FR[i];
					cumDR += DR[i];
				}

				if(cumOP == 0.0)	fTF = 9999.9;
				else	fTF = P_TF / P_OP[line];
				fSP = cumSP / cumTF;
				fFR = cumFR / cumTF;
				fDR = cumDR / cumTF;

				// ************************************
				// Calculation of Dxx of SP
				for(j=0; j<7; j++)	SP_d[j] = FR_d[j] = DR_d[j] = 0.0;

				if(fSP > 0.0001){
					tm_pre = tm = 0.0;
					a = 0.5;
					for(i=0; i<100; ++i){
						tm += (SP[i] / cumSP);

						b = (i+1)*0.1;

						SP_d[0] = 0.5;
						for(j=1; j<6; ++j){
							if(tm_pre < dth[j] && tm > dth[j]){
								SP_d[j] = a + (b - a) * (dth[j] - tm_pre)/(tm - tm_pre);
								//printf("D: %d, %f, %f, %f\n", j, tm, dth[j], SP_d[j]);
							}
						}
						if(tm > 0.99999999){
							SP_d[6] = a + 0.1;
							break;
						}

						if(SP[i] != 0.0) a = (i+1)*0.1;
						
						tm_pre = tm;
					}
				}

				// ************************************
				// Calculation of Dxx of FR
				if(fFR > 0.0001){
					tm_pre = tm = 0.0;
					a = 0.5;
					for(i=0; i<100; ++i){
						tm += (FR[i] / cumFR);

						b = (i+1)*0.1;

						FR_d[0] = 0.5;
						for(j=1; j<6; ++j){
							if(tm_pre < dth[j] && tm > dth[j]){
								FR_d[j] = a + (b - a) * (dth[j] - tm_pre)/(tm - tm_pre);

								if(FR_d[j] > Dmax_OP[line])	FR_d[j] = Dmax_OP[line];
								if(FR_d[j] > Dmax_TF)	FR_d[j] = Dmax_TF;
								//printf("D: %d, %d, %f, %f, %f\n", j, line, tm, Dmax_OP[line], FR_d[j]);
								
							}
						}
						if(tm > 0.99999999)	break;

						if(FR[i] != 0.0) a = (i+1)*0.1;

						tm_pre = tm;
					}
					FR_d[6] = Dmax_OP[line];
					if(Dmax_OP[line] > Dmax_TF)	FR_d[6] = Dmax_TF;
				}

				// ************************************
				// Calculation of Dxx of DR
				if(fDR > 0.0001){
					tm_pre = tm = 0.0;
					a = 0.5;
					if(fDR == 0.0){
						for(j=0; j<7; ++j)	DR_d[j] = 0.0;
					}
					else{
						DR_d[0] = 9999.9;
						for(i=0; i<100; ++i){
							tm += (DR[i] / cumDR);

							b = (i+1)*0.1;

							if(DR[i] != 0.0 && DR_d[0] == 9999.9){
								DR_d[0] = i*0.1;
							}
							for(j=1; j<6; ++j){
								if(tm_pre < dth[j] && tm > dth[j]){
									DR_d[j] = a + (b - a) * (dth[j] - tm_pre)/(tm - tm_pre);
									if(DR_d[j] > Dmax_TF)	DR_d[j] = Dmax_TF;
									//printf("D: %d, %f, %f, %f\n", j, tm, dth[j], DR_d[j]);
								}
							}
							if(tm > 0.99999999)	break;

							if(DR[i] != 0.0) a = (i+1)*0.1;

							tm_pre = tm;
						}
						DR_d[6] = Dmax_TF;
					}
				}

				if(fFR == 0.0) pp = 0.0;
				if(wei_a == 0.01) wei_a = 0.0;
				if(wei_b == 0.93) wei_b = 0.0;
				if(wei_a == 0.0 && wei_b == 0.0) SP_limit = -5;

				fprintf(fpDst, "%s,%s,%f,%f,%d,%d,%f,%f,%f,%f,%f,%f,%f,", DMY, ID, P_OP[line], P_TF, cnt_OP[line], cnt_TF, D50_OP[line], Dmax_OP[line], fTF, pp, wei_a, wei_b, SP_limit*0.1+0.1);
				fprintf(fpDst, "%f,%f,%f,%f,%f,%f", fSP, fFR, fDR, P_TF*fSP, P_TF*fFR, P_TF*fDR);
				
				for(j=0; j<7; ++j){
					if(cumSP == 0.0) fprintf(fpDst, ",NA");
					else fprintf(fpDst, ",%f", SP_d[j]);
				}
				for(j=0; j<7; ++j){
					if(cumFR == 0.0) fprintf(fpDst, ",NA");
					else fprintf(fpDst, ",%f", FR_d[j]);
				}		
				for(j=0; j<7; ++j){
					if(cumDR == 0.0) fprintf(fpDst, ",NA");
					else fprintf(fpDst, ",%f", DR_d[j]);
				}
				
				for(j=0; j<100; ++j)	fprintf(fpDst, ",%f", SP[j]);
				for(j=0; j<100; ++j)	fprintf(fpDst, ",%f", FR[j]);
				for(j=0; j<100; ++j)	fprintf(fpDst, ",%f", DR[j]);
				fprintf(fpDst, "\n");
				

			}
			

			fclose(fpDst);
			fclose(fpSrc);

			printf("\n");
		
		} while(FindNextFile(hFind2, &fd2));

		FindClose(hFind2);

	} while(FindNextFile(hFind, &fd));

	FindClose(hFind);

    if(E == 0){
		printf( "\n\n");
		printf( " Complete TF partitioning \n");
		printf( "====================\n\n");

		printf( " To terminate the program, key in something and 'Enter'.\n");
		scanf("%d", E);
        E = 0;
    }

}

