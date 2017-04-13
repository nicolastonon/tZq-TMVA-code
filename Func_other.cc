#include "Func_other.h"

using namespace std;

//Use stat function (from library sys/stat) to check if a file exists
bool Check_File_Existence(const TString& name)
{
  struct stat buffer;
  return (stat (name.Data(), &buffer) == 0); //true if file exists
}

//Move file with bash command 'mv'
void MoveFile(TString origin_path, TString dest_path)
{
	TString command = "mv "+origin_path + " " + dest_path;
	system(command.Data() );

	return;
}

//Copy file with bash command 'cp'
void CopyFile(TString origin_path, TString dest_path)
{
	TString command = "cp "+origin_path + " " + dest_path;
	system(command.Data() );

	return;
}

void Load_Canvas_Style()
{
	// For the canvas:
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasColor(0); // must be kWhite but I dunno how to do that in PyROOT
	gStyle->SetCanvasDefH(600); //Height of canvas
	gStyle->SetCanvasDefW(600); //Width of canvas
	gStyle->SetCanvasDefX(0);   //POsition on screen
	gStyle->SetCanvasDefY(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadColor(0); // kWhite
	gStyle->SetPadGridX(0); //false
	gStyle->SetPadGridY(0); //false
	gStyle->SetGridColor(0);
	gStyle->SetGridStyle(3);
	gStyle->SetGridWidth(1);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(1);
	gStyle->SetFrameFillColor(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameLineColor(1);
	gStyle->SetFrameLineStyle(1);
	gStyle->SetFrameLineWidth(1);
	gStyle->SetHistLineColor(1);
	gStyle->SetHistLineStyle(0);
	gStyle->SetHistLineWidth(1);
	gStyle->SetEndErrorSize(2);
	gStyle->SetOptFit(1011);
	gStyle->SetFitFormat("5.4g");
	gStyle->SetFuncColor(2);
	gStyle->SetFuncStyle(1);
	gStyle->SetFuncWidth(1);
	gStyle->SetOptDate(0);
	gStyle->SetOptFile(0);
	gStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	gStyle->SetStatColor(0); // kWhite
	gStyle->SetStatFont(42);
	gStyle->SetStatFontSize(0.04);
	gStyle->SetStatTextColor(1);
	gStyle->SetStatFormat("6.4g");
	gStyle->SetStatBorderSize(1);
	gStyle->SetStatH(0.1);
	gStyle->SetStatW(0.15);
	gStyle->SetPadTopMargin(0.07);
	gStyle->SetPadBottomMargin(0.13);
	gStyle->SetPadLeftMargin(0.16);
	gStyle->SetPadRightMargin(0.03);
	gStyle->SetOptTitle(0);
	gStyle->SetTitleFont(42);
	gStyle->SetTitleColor(1);
	gStyle->SetTitleTextColor(1);
	gStyle->SetTitleFillColor(10);
	gStyle->SetTitleFontSize(0.05);
	gStyle->SetTitleColor(1, "XYZ");
	gStyle->SetTitleFont(42, "XYZ");
	gStyle->SetTitleSize(0.06, "XYZ");
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYOffset(1.25);
	gStyle->SetLabelColor(1, "XYZ");
	gStyle->SetLabelFont(42, "XYZ");
	gStyle->SetLabelOffset(0.007, "XYZ");
	gStyle->SetLabelSize(0.05, "XYZ");
	gStyle->SetAxisColor(1, "XYZ");
	gStyle->SetStripDecimals(1); // kTRUE
	gStyle->SetTickLength(0.03, "XYZ");
	gStyle->SetNdivisions(510, "XYZ");
	gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	gStyle->SetPadTickY(1);
	gStyle->SetOptLogx(0);
	gStyle->SetOptLogy(0);
	gStyle->SetOptLogz(0);
	gStyle->SetPaperSize(20.,20.);
}

//Convert a float into a TString
TString Convert_Number_To_TString(float number)
{
	stringstream ss;
	ss << number;
	TString ts = ss.str();
	return ts;
}

//Convert a TString into a float
float Convert_TString_To_Number(TString ts)
{
	float number = 0;
	string s = ts.Data();
	stringstream ss(s);
	ss >> number;
	return number;
}

//Find a number into a TString, and returns it as a float
float Find_Number_In_TString(TString ts)
{
	TString tmp = ""; int number = 0;
	string s = ts.Data(); int ts_size = s.size(); //Only to get TString size

	for (int i=0; i < ts_size; i++)
	{
		if( isdigit(ts[i]) )
		{
			do
			{
				tmp += ts[i];
				i++;
			} while(isdigit(ts[i]) || ts[i] == '.'); //NB : Pay attention to quotes : "a" creates a 2-char array (letter+terminator) -> string. 'a' identifies a single character !

		  break;
		}
	}

	return Convert_TString_To_Number(tmp);
}

//Translates signs into words
TString Convert_Sign_To_Word(TString ts_in)
{
	TString sign = "";

	if(ts_in.Contains("<=")) {sign = "MaxEq";}
	else if(ts_in.Contains(">=")) {sign = "MinEq";}
	else if(ts_in.Contains(">")) {sign = "Min";}
	else if(ts_in.Contains("<")) {sign = "Max";}
	else if(ts_in.Contains("==")) {sign = "Eq";}

	return sign;
}

//Used when a "cut" is composed of 2 conditions -> breaks it into a pair of TStrings <cut1,cut2>
pair<TString,TString> Break_Cuts_In_Two(TString multiple_cut)
{
	TString cut1 = "", cut2 = "";

	string s = multiple_cut.Data(); int size = s.size(); //To get TString size

	for (int i=0; i < size; i++) //Extract condition 1
	{
		if(multiple_cut[i] == '&' || multiple_cut[i] == '|') {break;} //stop when we find '&&' or '||'
		if(multiple_cut[i] != ' ') {cut1 += multiple_cut[i];} //else, store characters in TString
	}

	TString tmp = "";
	for (int i=0; i < size; i++) //Extract condition 2
	{
		tmp+= multiple_cut[i];
		if( (!tmp.Contains("&&") && !tmp.Contains("||")) || multiple_cut[i] == '&' || multiple_cut[i] == '|') {continue;}
		if(multiple_cut[i] != ' ') {cut2 += multiple_cut[i];}
	}

	pair<TString,TString> the_cuts; the_cuts.first = cut1; the_cuts.second = cut2;

	return the_cuts;
}

//Make sure that systematic name follows Combine conventions
TString Combine_Naming_Convention(TString name)
{
	if(name.Contains("Up") || name.Contains("Down") ) {return name;} //If systematics is already following Combine convention

	else if(name.Contains("__plus"))
	{
		int i = name.Index("__plus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Up"; //Add Combine syst. suffix
	}

	else if(name.Contains("__minus"))
	{
		int i = name.Index("__minus"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "Down"; //Add Combine syst. suffix
	}

	return name;
}

//Make sure that systematic name follows Theta conventions

TString Theta_Naming_Convention(TString name)
{
	if(name.Contains("__plus") || name.Contains("__minus") ) {return name;} //If systematics is already following Theta convention

	else if(name.Contains("Up"))
	{
		int i = name.Index("Up"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "__plus"; //Add Theta syst. suffix
	}

	else if(name.Contains("Down"))
	{
		int i = name.Index("Down"); //Find index of substring
		name.Remove(i); //Remove substring
		name+= "__minus"; //Add Theta syst. suffix
	}

	return name;
}

//Modifies the TMVA standard output to extract only the useful ranking information (the hard-coded way)
void Extract_Ranking_Info(TString TMVA_output_file, TString bdt_type, TString channel)
{
	ifstream file_in(TMVA_output_file.Data() );

	ofstream file_out("ranking_tmp.txt");

	file_out<<"--- "<<bdt_type<<" "<<channel<<" : Variable Ranking ---"<<endl<<endl;

	string string_start = "Ranking input variables (method specific)...";
	string string_stop = "----------------------------------------------";

	string line;

	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(line.find(string_start) != std::string::npos) {break;} //When 'string_start' is found, start copying lines into output file
	}

	int k=0;
	while(!file_in.eof( ) )
	{
		getline(file_in, line);

		if(k>3) //We know that the first 3 lines are useless
		{
			if(line.find(string_stop) != std::string::npos) {break;} //Don't stop until all variables are read

			TString string_to_write = line;
			int index = string_to_write.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		    string_to_write.Remove(0,index);

		    string s = string_to_write.Data(); int size = s.size(); //Compute size of the TString this way (ugly)
		    for(int i=0; i<size; i++)
		    {
		        if(isdigit(string_to_write[i]) ) //Only copy what is after the ranking index, remove the rest
		        {
		            string_to_write.Remove(0,i);
		            break;
		        }
		    }

			file_out<<string_to_write.Data()<<endl;
		}

		k++;
	}

	TString mv_command = "mv ranking_tmp.txt " + TMVA_output_file;
	system(mv_command.Data() ); //Replace TMVA output textfile by modified textfile !

	return;
}

//Read modified TMVA ranking file in order to fill 2 vectors (passed by reference) ordered by decreasing rank (var names & importances)
//NB : Hard-coded = ugly !
void Get_Ranking_Vectors(TString bdt_type, TString channel, vector<TString> &v_BDTvar_name, vector<double> &v_BDTvar_importance)
{
	TString file_rank_path = "./outputs/Rankings/"+bdt_type+"/RANKING_"+bdt_type+"_"+channel+".txt";
	if(!Check_File_Existence(file_rank_path.Data()) ) {std::cout<<file_rank_path<<" not found -- Abort"<<endl; return;}
	ifstream file_in(file_rank_path);

	string line;
	getline(file_in, line); getline(file_in, line);

	while(!file_in.eof())
	{
		getline(file_in, line);

		TString ts = line; //Convert to TString for easy modification

		int index = ts.First(":"); //Find first occurence of ':' in the lines, and remove the text before it (useless)
		ts.Remove(0,index+1); //Remove first useless characters

		TString var_name = "";
		string ts_tmp = ts.Data(); int size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(var_name != "" && ts[ichar] == ' ') {break;} //end of var name
			else if(ts[ichar] == ' ') {continue;} //Begin at var name

			var_name+=ts[ichar];
		}

		v_BDTvar_name.push_back(var_name);


		index = ts.First(":"); //Find other occurence of ':' (placed before variable importance number)
		ts.Remove(0,index+1); //After that, only the importance number remains in the TString

 		TString var_importance_string = "";
 		ts_tmp = ts.Data(); size = ts_tmp.size();
		for(int ichar=0; ichar<size; ichar++)
		{
			if(ts[ichar] == ' ') {continue;}
			var_importance_string+=ts[ichar];
		}

		//Convert tstring to number (importance of the var)
		double var_importance = 0;
		if(var_importance_string.Contains("e-01") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.1;
		}
		else if(var_importance_string.Contains("e-02") )
		{
			index = var_importance_string.Index('e');
			var_importance_string.Remove(index);
			var_importance = Convert_TString_To_Number(var_importance_string);
			var_importance*= 0.01;
		}

		v_BDTvar_importance.push_back(var_importance);
	}


	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_BDTvar_name.erase(v_BDTvar_name.begin() + v_BDTvar_name.size()-1);
	v_BDTvar_importance.erase(v_BDTvar_importance.begin() + v_BDTvar_importance.size()-1);

	return;
}

//Order the variables from file created via BDT-optimization sectrion of the main(), by decreasing significance LOSS caused by the removal of corresponding Variable
//--> the variable which affects the most the final significance will be ranked first
void Order_BDTvars_By_Decreasing_Signif_Loss(TString file_path)
{
	ifstream file_in(file_path.Data() );

	ofstream file_out( (file_path+"_tmp") );

	vector<TString> v_variables; vector<float> v_signif;

	string line; TString ts;
	int count_line=0;
	while(!file_in.eof() )
	{
		getline(file_in, line); count_line++; ts = line;
		if(count_line<7) {file_out<<ts<<endl; continue;}

		TString var_name_tmp;
		for(int ichar=0; ichar<line.size(); ichar++)
		{
			if(ts[ichar] != ' ') {var_name_tmp+= ts[ichar];}
			else {break;}
		}

		v_variables.push_back(var_name_tmp);

		int index = ts.First(">");
		ts.Remove(0, index+2);

		v_signif.push_back(Convert_TString_To_Number(ts) );
	}
	file_in.close();

	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_variables.erase(v_variables.begin() + v_variables.size()-1);
	v_signif.erase(v_signif.begin() + v_signif.size()-1);


	//--- Ordering
	vector<TString> v_variables_ordered; vector<float> v_signif_ordered;

	float signif_min = 999; int index_signif_min = 999;

	while(v_variables.size() != 0)
	{
		signif_min = 999; index_signif_min = 999;

		for(int i=0; i<v_signif.size(); i++)
		{
			if(v_signif[i] < signif_min)
			{
				signif_min = v_signif[i];
				index_signif_min = i;
			}
		}

		v_variables_ordered.push_back(v_variables[index_signif_min]);
		v_signif_ordered.push_back(v_signif[index_signif_min]);

		v_variables.erase(v_variables.begin() + index_signif_min);
		v_signif.erase(v_signif.begin() + index_signif_min);
	}

	//--- Write ordered vars into output file
	for(int i=0; i<v_variables_ordered.size(); i++)
	{
		file_out<<v_variables_ordered[i]<<" "<<v_signif_ordered[i]<<endl;
	}

	//--- Replace old file by new one
	MoveFile( (file_path+"_tmp"), file_path );


	return;
}


//Order the sets of cuts from file created via cut-optimization section of the main(), by decreasing significance
void Order_Cuts_By_Decreasing_Signif_Loss(TString file_path)
{
	ifstream file_in(file_path.Data() );

	ofstream file_out( (file_path+"_tmp") );

	vector<TString> v_cuts; vector<float> v_signif;

	string line; TString ts;
	int count_line=0;
	while(!file_in.eof() )
	{
		getline(file_in, line); count_line++; ts = line;
		if(count_line<10) {file_out<<ts<<endl; continue;} //first lines useless

		TString cut_name_tmp;
		for(int ichar=0; ichar<line.size(); ichar++)
		{
			if(ts[ichar] != ' ') {cut_name_tmp+= ts[ichar];}
			else {break;}
		}

		v_cuts.push_back(cut_name_tmp);

		int index = ts.Index("->");
		ts.Remove(0, index+3);

		v_signif.push_back(Convert_TString_To_Number(ts) );
	}
	file_in.close();

	//Because while loop on 'eof' --> one line too many at the end --> erase last entry
	v_cuts.erase(v_cuts.begin() + v_cuts.size()-1);
	v_signif.erase(v_signif.begin() + v_signif.size()-1);


	//--- Ordering
	vector<TString> v_cuts_ordered; vector<float> v_signif_ordered;

	float signif_max = -999; int index_signif_max = -999;

	while(v_cuts.size() != 0)
	{
		signif_max = -999; index_signif_max = -999;

		for(int i=0; i<v_signif.size(); i++)
		{
			if(v_signif[i] > signif_max)
			{
				signif_max = v_signif[i];
				index_signif_max = i;
			}
		}

		v_cuts_ordered.push_back(v_cuts[index_signif_max]);
		v_signif_ordered.push_back(v_signif[index_signif_max]);

		v_cuts.erase(v_cuts.begin() + index_signif_max);
		v_signif.erase(v_signif.begin() + index_signif_max);
	}

	//--- Write ordered vars into output file
	for(int i=0; i<v_cuts_ordered.size(); i++)
	{
		file_out<<v_cuts_ordered[i]<<" "<<v_signif_ordered[i]<<endl;
	}

	//--- Replace old file by new one
	MoveFile( (file_path+"_tmp"), file_path );


	return;
}
