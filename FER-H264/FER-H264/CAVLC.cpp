#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

inline int ABS(const int a)
{
	if (a>0)
		return a;
	else
		return -a;
}

char zigZagTablica[4][4]={
0,1,5,6,
2,4,7,12,
3,8,11,13,
9,10,14,15
};

char me_table_03[16][2]={
{15 , 0},
{0 , 1},
{7 , 2},
{11 , 4},
{13 , 8},
{14 , 3},
{3 , 5},
{5 , 10},
{10 , 12},
{12 , 15},
{1 , 7},
{2 , 11},
{4 , 13},
{8 , 14},
{6 , 6},
{9 , 9}
};

char me_table_12[48][2]={
{47 , 0},
{31 , 16},
{15 , 1},
{0 , 2},
{23 , 4},
{27 , 8},
{29 , 32},
{30 , 3},
{7 , 5},
{11 , 10},
{13 , 12},
{14 , 15},
{39 , 47},
{43 , 7},
{45 , 11},
{46 , 13},
{16 , 14},
{3 , 6},
{5 , 9},
{10 , 31},
{12 , 35},
{19 , 37},
{21 , 42},
{26 , 44},
{28 , 33},
{35 , 34},
{37 , 36},
{42 , 40},
{44 , 39},
{1 , 43},
{2 , 45},
{4 , 46},
{8 , 17},
{17 , 18},
{18 , 20},
{20 , 24},
{24 , 19},
{6 , 21},
{9 , 26},
{22 , 28},
{25 , 23},
{32 , 27},
{33 , 29},
{34 , 30},
{36 , 22},
{40 , 25},
{38 , 38},
{41 , 41}
};

string run_before[15][7]={
{ "1" , "1" , "11" , "11" , "11" , "11" , "111"},
{ "0" , "01" , "10" , "10" , "10" , "000" , "110"},
{ "-" , "00" , "01" , "01" , "011" , "001" , "101"},
{ "-" , "-" , "00" , "001" , "010" , "011" , "100"},
{ "-" , "-" , "-" , "000" , "001" , "010" , "011"},
{ "-" , "-" , "-" , "-" , "000" , "101" , "010"},
{ "-" , "-" , "-" , "-" , "-" , "100" , "001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "0001"},
{ "-" , "-" , "-" , "-" , "-" , "00001" , "0001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "000001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "0000001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "00000001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "000000001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "0000000001"},
{ "-" , "-" , "-" , "-" , "-" , "-" , "00000000001"}
};

string total_zeros_DC_4x4[8][7]={
{ "1" , "000" , "000" , "110" , "00" , "00" , "0"},
{ "010" , "01" , "001" , "00" , "01" , "01" , "1"},
{ "011" , "001" , "01" , "01" , "10" , "1" , "-"},
{ "0010" , "100" , "10" , "10" , "11" , "-" , "-"},
{ "0011" , "101" , "110" , "111" , "-" , "-" , "-"},
{ "0001" , "110" , "111" , "-" , "-" , "-" , "-"},
{ "00001" , "111" , "-" , "-" , "-" , "-" , "-"},
{ "00000" , "-" , "-" , "-" , "-" , "-" , "-"}
};

string total_zeros_DC_2x2[4][3]={
{"1","1","1"},
{"01","01","0"},
{"001","00","-"},
{"000","-","-"}
};

string total_zeros_8_15[9][6]={
{ "000001" , "000001" , "00001" , "0000" , "0000" , "000"},
{ "0001" , "000000" , "00000" , "0001" , "0001" , "001"},
{ "00001" , "0001" , "001" , "001" , "01" , "1"},
{ "011" , "11" , "11" , "010" , "1" , "01"},
{ "11" , "10" , "10" , "1" , "001" , "-"},
{ "10" , "001" , "01" , "011" , "-" , "-"},
{ "010" , "01" , "0001" , "-" , "-" , "-"},
{ "001" , "00001" , "-" , "-" , "-" , "-"},
{ "000000" , "-" , "-" , "-" , "-" , "-"}
};

string total_zeros_1_7[16][6]={
{ "1" , "111" , "0101" , "00011" , "0101" , "000001"},
{ "011" , "110" , "111" , "111" , "0100" , "00001"},
{ "010" , "101" , "110" , "0101" , "0011" , "111"},
{ "0011" , "100" , "101" , "0100" , "111" , "110"},
{ "0010" , "011" , "0100" , "110" , "110" , "101"},
{ "00011" , "0101" , "0011" , "101" , "101" , "100"},
{ "00010" , "0100" , "100" , "100" , "100" , "011"},
{ "000011" , "0011" , "011" , "0011" , "011" , "010"},
{ "000010" , "0010" , "0010" , "011" , "0010" , "0001"},
{ "0000011" , "00011" , "00011" , "0010" , "00001" , "001"},
{ "0000010" , "00010" , "00010" , "00010" , "0001" , "000000"},
{ "00000011" , "000011" , "000001" , "00001" , "00000" , "-"},
{ "00000010" , "000010" , "00001" , "00000" , "-" , "-"},
{ "000000011" , "000001" , "000000" , "-" , "-" , "-"},
{ "000000010" , "000000" , "-" , "-" , "-" , "-"},
{ "000000001" , "-" , "-" , "-" , "-" , "-"}
};

//Standard, str 234-235
string coeff_token_table[4][17][4]={
"1","11","1111","0000",
"000101","001011","001111","0000",
"00000111","000111","001011","0001",
"000000111","0000111","001000","0010",
"0000000111","00000111","0001111","0011",
"00000000111","00000100","0001011","0100",
"0000000001111","000000111","0001001","0101",
"0000000001011","00000001111","0001000","0110",
"0000000001000","00000001011","00001111","011100",
"00000000001111","000000001111","00001011","100000",
"00000000001011","000000001011","000001111","100100",
"000000000001111","000000001000","000001011","101000",
"000000000001011","0000000001111","000001000","101100",
"0000000000001111","0000000001011","0000001101","110000",
"0000000000001011","0000000000111","0000001001","110100",
"0000000000000111","00000000001001","0000000101","111000",
"0000000000000100","00000000000111","0000000001","111100",
"","","","",
"01","10","1110","0000",
"000100","00111","01111","0001",
"00000110","001010","01100","0010",
"000000110","000110","01010","0011",
"0000000110","0000110","01000","0100",
"00000000110","00000110","001110","0101",
"0000000001110","000000110","001010","0110",
"0000000001010","00000001110","0001110","011101",
"00000000001110","00000001010","00001110","100001",
"00000000001010","000000001110","00001010","100101",
"000000000001110","000000001010","000001110","101001",
"000000000001010","0000000001110","000001010","101101",
"000000000000001","0000000001010","000000111","110001",
"0000000000001110","00000000001011","0000001100","110101",
"0000000000001010","00000000001000","0000001000","111001",
"0000000000000110","00000000000110","0000000100","111101",
"","","","",
"","","","",
"001","011","1101","0001",
"0000101","001001","01110","0010",
"00000101","000101","01011","0011",
"000000101","0000101","01001","0100",
"0000000101","00000101","001101","0101",
"00000000101","000000101","001001","0110",
"0000000001101","00000001101","0001101","011110",
"0000000001001","00000001001","0001010","100010",
"00000000001101","000000001101","00001101","100110",
"00000000001001","000000001001","00001001","101010",
"000000000001101","0000000001101","000001101","101110",
"000000000001001","0000000001001","000001001","110010",
"0000000000001101","0000000000110","0000001011","110110",
"0000000000001001","00000000001010","0000000111","111010",
"0000000000000101","00000000000101","0000000011","111110",
"","","","",
"","","","",
"","","","",
"00011","0101","1100","0010",
"000011","0100","1011","0011",
"0000100","00110","1010","0100",
"00000100","001000","1001","0101",
"000000100","000100","1000","011011",
"0000000100","0000100","01101","011111",
"00000000100","000000100","001100","100011",
"0000000001100","00000001100","0001100","100111",
"00000000001100","00000001000","00001100","101011",
"00000000001000","000000001100","00001000","101111",
"000000000001100","0000000001100","000001100","110011",
"000000000001000","0000000001000","0000001010","110111",
"0000000000001100","0000000000001","0000000110","111011",
"0000000000001000","00000000000100","0000000010","111111"
};

enum expGolombMode {ue=1, se, me, te};

enum macroBlockPrediction {intra_4x4=1, intra_8x8, inter};

enum teRange {bits, fullRange};

int binaryToDecimal(string bitString)
{
	int result=0;
	for (unsigned int i=0;i<bitString.length();i++)
	{
		result=(result<<1)+bitString[i];
	}
	return result;
}

string decimalToBinary(int decimalNumber)
{
	string result="";
	while (decimalNumber>0)
	{
		result+=(char)((decimalNumber%2)+48);
	}

	//Okreni string
	char tempC;
	for (unsigned int i=0;i<result.length()/2;i++)
	{
		tempC=result[i];
		result[i]=result[result.length()-i-1];
		result[result.length()-i-1]=tempC;
	}

	return result;
}

string expGolomb_Encode(int codeNum)
{
	string result="";

	//Izracun izraza M=log2(codeNum+1)
	int temp=codeNum+1;
	int M=0;
	while (temp>0)
	{
		temp=temp>>1;
		M++;
	}

	string INFO=decimalToBinary(codeNum+1-(2<<M));

	return result;

}

int expGolomb_Decode(string bitString, expGolombMode mode,int chromaArrayType,macroBlockPrediction mbPrediction,teRange range)
{
	int leadingZeros=0,codeNum=0;
	for (unsigned int i=0;i<bitString.length();i++)
	{
		if (bitString[i]=='1' && leadingZeros==0)
			leadingZeros=i;
		break;
	}

	if (leadingZeros=0)
	{
		codeNum=0;
	}
	else
	{
		codeNum=(1<<leadingZeros)-1+binaryToDecimal(bitString.substr(leadingZeros+1));
	}

	switch(mode)
	{
		case ue:
			return codeNum;
		case se:
			return (-1)*((codeNum+1)%2)*(codeNum+1)/2;
		case me:
			if ((chromaArrayType==1) || (chromaArrayType==2))
			{
				return me_table_12[codeNum][mbPrediction];
			}
			else
			{
				return me_table_03[codeNum][mbPrediction];
			}
		case te:
			if (range==bits)
				return !(bitString[0]);
			else
				return codeNum;
			
		//Ne smije se dogoditi
		default:
			cout << "EXP_GOLOMB default mode nije dozvoljen. Mode:" << mode << endl;
			return codeNum;
	}
}


/*
CAVLC entropijsko kodiranje blokova od 4x4 koeficijenata.

Chroma DC 2x2 block (4:2:0 chroma sampling) i Chroma DC 2x4 block (4:2:2 chroma sampling) nisu podržani.
Njihove prilagođene total_zeros tablice su na 239. strani norme.


*/
string CAVLC_4x4_Encode(int nA, int nB, char tablica[4][4])
{
	int totalCoeffs=0,trailingOnes=0,nC=0;
	string rezultat="";
	string T1Predznaci="";
	bool trailing_ones_sign_flag[3];
	int runBefore[16],run[16];
	char level[16],coeffLevel[16];
	for (int i=0;i<16;i++)
	{
		runBefore[i]=0;
		run[i]=0;
		coeffLevel[i]=0;
	}

	//za ulaz 4x4
	int brElem=16;

	//Standard, str 233
	if (nA==-1 || nB ==-1)
	{
		nC=nA+nB;
	}
	else
	{
		nC=(nA+nB+1)>>1;
	}

	//Stvori zigzag niz iz tablice i izbroj koeficijente, postavi runbefore niz
	char zigZagNiz[16];
	char coeffArray[16];
	int nuleIspredKoeficijenta=0;
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
		{
			zigZagNiz[zigZagTablica[i][j]]=tablica[i][j];
			if (zigZagNiz[zigZagTablica[i][j]]==0)
			{
				nuleIspredKoeficijenta++;
			}
			else
			{
				coeffArray[totalCoeffs++]=tablica[i][j];
				runBefore[totalCoeffs++]=nuleIspredKoeficijenta;
				nuleIspredKoeficijenta=0;
			}
		}
	
	//Izbroji koliko ima +/-1 elemenata na kraju niza i zapiši predznake u T1Predznake (obrnuti redoslijed)
	for (int i=15;i>=0;i--)
	{
		if (zigZagNiz[i]==1 || zigZagNiz[i]==-1)
		{
			T1Predznaci+=((zigZagNiz[i]==-1)?"-1":"1");
			trailing_ones_sign_flag[trailingOnes]=(zigZagNiz[i]==-1);
			trailingOnes++;
		}
		else if (zigZagNiz[i]!=0)
			break;
	}

	//Ovisno o broju koeficijenata i jedinica na kraju niza, odredi kodnu riječ.
	if (nC>=0 && nC<2)
	{
		rezultat+=coeff_token_table[trailingOnes][totalCoeffs][0];
	}
	else if (nC>=2 && nC<4)
	{
		rezultat+=coeff_token_table[trailingOnes][totalCoeffs][1];
	}
	else if (nC>=4 && nC<8)
	{
		rezultat+=coeff_token_table[trailingOnes][totalCoeffs][2];
	}
	else if (nC>=8)
	{
		rezultat+=coeff_token_table[trailingOnes][totalCoeffs][3];
	}

	//Ubaci kodove predznaka trailing jedinica
	rezultat+=T1Predznaci;

	//Standard, str 78, kodiranje koeficijenata
	int suffixLength=0;
	string tempStr="";
	int brojacTrailingOnes=0,levelCode=0,level_prefix,level_suffix;
	unsigned int spBorder;
	if (totalCoeffs>0)
	{
		if (totalCoeffs>10 && trailingOnes<3)
		{
			suffixLength=1;
		}
		else
		{
			suffixLength=0;
		}
		for (int i=0;i<totalCoeffs;i++)
		{
			if (i<trailingOnes)
			{
				level[i]=1-2*trailing_ones_sign_flag[brojacTrailingOnes++];
			}
			else
			{
				tempStr=expGolomb_Encode(coeffArray[i]);

				//Dodavanje kôda pojedinog level-a na konačni bitstream
				rezultat+=tempStr;
				spBorder=tempStr.find('1');
				level_prefix=spBorder;
				level_suffix=binaryToDecimal(tempStr.substr(spBorder));
				suffixLength=spBorder; //TODO: SuffixLength==PrefixLength?
				levelCode=((15<level_prefix)?15:level_prefix)<<suffixLength;
				if (suffixLength>0 || level_prefix>=14)
				{
					levelCode+=level_suffix;
				}
				if (level_prefix>=15 && suffixLength==0)
				{
					levelCode+=15;
				}
				if (level_prefix>=16)
				{
					levelCode+=(1<<(level_prefix-3))-4096;
				}
				if (i==trailingOnes && trailingOnes<3)
				{
					levelCode+=2;
				}
				if (levelCode%2 == 0)
				{
					level[i]=(levelCode+2)>>1;
				}
				else
				{
					level[i]=(-levelCode-1)>>1;
				}
				if (suffixLength==0)
				{
					suffixLength=1;
				}
				if (ABS(level[i])>(3<<(suffixLength-1)) && suffixLength<6)
					suffixLength++;
			}
		}
		int zerosLeft,coeffNum;
		if (totalCoeffs<brElem)
		{
			zerosLeft=brElem-totalCoeffs;
		}
		else
		{
			zerosLeft=0;
		}

		if (totalCoeffs<7)
		{
			rezultat+=total_zeros_1_7[zerosLeft][totalCoeffs];
		}
		else
		{
			rezultat+=total_zeros_8_15[zerosLeft][totalCoeffs];
		}


		for (int i=0;i<totalCoeffs-1;i++) //TODO: treba li -1?
		{
			if (zerosLeft>0)
			{
				if (zerosLeft<7)
				{
					rezultat+=run_before[runBefore[i]][zerosLeft];
				}
				else
				{
					rezultat+=run_before[runBefore[i]][6];
				}
				run[i]=runBefore[i];
			}
			else
			{
				run[i]=0;
			}
			zerosLeft=zerosLeft-run[i];
		}
		run[totalCoeffs-1]=zerosLeft;
		coeffNum=-1;
		for (int i=totalCoeffs-1;i>=0;i--)
		{
			coeffNum+=run[i]+1;
			coeffLevel[coeffNum]=level[i];
		}
	}
	return rezultat;

}

int _tmain(int argc, _TCHAR* argv[])
{

//Formatiranje coeff tablice, prvi i drugi dio
/*
	ofstream iz2;
	iz2.open("tablica-formatirano.txt");
	for (int i=0;i<4;i++)
		for (int j=0;j<17;j++)
		{
			for (int k=0;k<4;k++)
			{
				iz2 << "\"" << coeff_token_table[i][j][k] << "\",";
			}
			iz2 << endl;
		}
	iz2.close();
	ulaz.close();

	ifstream ulaz;
	ofstream iz;
	iz.open("tablica-neformatirano.txt");
	string linija;
	string a[6];
	ulaz.open("tablica.txt");
	while (getline(ulaz,linija))
	{
			istringstream parser(linija);
			for (int i=0;i<6;i++)
				parser>>a[i];
			for (int i=2;i<6;i++)
				iz << "coeff_token_table[" << a[0] << "][" << a[1] << "][" << i-2 << "]=\"" << a[i] << "\";" <<  endl;
	}
	iz.close();
	ulaz.close();
	*/

	
	ifstream ulaz;
	ofstream iz;
	iz.open("ZERO.txt");
	string linija;
	string a[8];
	ulaz.open("totalzeros5.txt");
	while (getline(ulaz,linija))
	{
			istringstream parser(linija);
			for (int i=0;i<8;i++)
				parser>>a[i];
			iz << "{ \"" << a[1] << "\" , \"" << a[2] << "\" , \"" << a[3] <<"\" , \"" << a[4] << "\" , \"" << a[5] << "\" , \"" << a[6] << "\" , \"" << a[7] << "\"}," << endl;
	}
	iz.close();
	ulaz.close();

	return 0;
}

