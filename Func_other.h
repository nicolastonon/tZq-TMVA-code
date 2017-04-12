#ifndef Func_other_h
#define Func_other_h

#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h> // to be able to check file existence

#include "TStyle.h"


	void Load_Canvas_Style();
	TString Convert_Number_To_TString(float);
	float Convert_TString_To_Number(TString);
	float Find_Number_In_TString(TString);
	TString Convert_Sign_To_Word(TString);
	std::pair<TString,TString> Break_Cuts_In_Two(TString);
	TString Combine_Naming_Convention(TString);
	TString Theta_Naming_Convention(TString);
	void Extract_Ranking_Info(TString, TString, TString);
	void Get_Ranking_Vectors(TString, TString, std::vector<TString>&, std::vector<double>&);
	bool Check_File_Existence(const std::string&);
	void MoveFile(TString, TString);
	void CopyFile(TString, TString);



#endif
