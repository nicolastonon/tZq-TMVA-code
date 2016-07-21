#    _____ __  ____     ___       ____ ___  ____  _____ 
#   |_   _|  \/  \ \   / / \     / ___/ _ \|  _ \| ____|
#     | | | |\/| |\ \ / / _ \   | |  | | | | | | |  _|  
#     | | | |  | | \ V / ___ \  | |__| |_| | |_| | |___ 
#     |_| |_|  |_|  \_/_/   \_\  \____\___/|____/|_____|
#                                                       
_____________________________________________________________________________
* This GitHub repository contains the BDT-framework used in the tZq-SM analysis @ 13 TeV.

* The files are : 
    - theMVAtool.h & theMVAtool.cc --- this is a class which contains all the methods necessary to the BDT procedure (training, testing, reading = producing BDT templates, creating control trees/histos/plots, generating pseudo-data to stay blind before pre-approval, etc.) ;
    - BDT_analysis.cc --- contains the "int main()" function. Most things can be configured from there (samples, channels, variables, systematics, function calls) ;
    - Makefile --- compiles everything into "./main" ;
    - Func_other.h --- can be used to store secondary functions
_____________________________________________________________________________




_____________________________________________________________________________
#        ___           ___          _____    
#       /  /\         /__/\        /  /::\   
#      /  /:/_        \  \:\      /  /:/\:\  
#     /  /:/ /\        \  \:\    /  /:/  \:\ 
#    /  /:/ /:/_   _____\__\:\  /__/:/ \__\:|
#   /__/:/ /:/ /\ /__/::::::::\ \  \:\ /  /:/
#   \  \:\/:/ /:/ \  \:\~~\~~\/  \  \:\  /:/ 
#    \  \::/ /:/   \  \:\  ~~~    \  \:\/:/  
#     \  \:\/:/     \  \:\         \  \::/   
#      \  \::/       \  \:\         \__\/    
#       \__\/         \__\/                  
