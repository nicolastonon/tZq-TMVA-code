echo "-------------------------"
echo "#WARNING# : GOING TO MERGE *ALL* READER FILES IN ./OUTPUTS/ !"
echo "---> Make sure there are only the template files you want in this directory !!"
echo ""
#hadd -f outputs/Combine_Input.root outputs/Reader_BDT_NJetsMin1Max4_NBJetsEq1.root outputs/Reader_BDTttZ_NJetsMin1_NBJetsMin1.root outputs/Reader_mTW_NJetsMin0_NBJetsEq0.root
hadd -f outputs/Combine_Input.root outputs/Reader_BDT*_NJetsMin1Max4_NBJetsEq1.root outputs/Reader_BDTttZ*_NJetsMin1_NBJetsMin1.root outputs/Reader_mTW*_NJetsMin0_NBJetsEq0.root
echo "-------------------------"
