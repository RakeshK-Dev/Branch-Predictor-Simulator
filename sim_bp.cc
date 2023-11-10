#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <inttypes.h>
#include <math.h>
#include <cmath>

#include <sstream>
#include <fstream>
#include <bits/stdc++.h>
#include <bitset>
#include <stdint.h>

#include "sim_bp.h"


using namespace std;

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/

/*struct PC_addr
{
    int val;
    PC_addr() : val(2) {}
};

struct BHR_table
{
    int val;
    BHR_table() : val(2) {}
};*/

uint32_t BHR_val = 0;
unsigned long int misprediction = 0;
unsigned long int prediction = 0;

//////////////////  Parsing Logic  //////////////////////
uint32_t Shifter (int x)
{
    int i = x, shift = 1;
    for (;i>0;i--)
    {
	shift *= 2;
    }
    shift -= 1;
    return shift;
}
//////////////////////////////////////////////////////////

uint32_t calculate_bimodal (u_int32_t Index, uint32_t Idx_shift_B, char outcome, uint32_t Predictor_B[], unsigned long int addr)
{
    //printf("BP: %d %d\n", Index, Predictor_B[Index]);
    if (outcome == 't' && Predictor_B[Index] < 3)
    {
        Predictor_B[Index] += 1;
        //printf("%d %d\n", Index, Predictor_B[Index]);
    }
    else if (outcome == 't')
    {
        //printf("%d %d\n", Index, Predictor_B[Index]);
    }
    else if (outcome == 'n' && Predictor_B[Index] > 0)
    {
        Predictor_B[Index] -= 1;
        //printf("%d %d\n", Index, Predictor_B[Index]);
    }
    else if (outcome == 'n')
    {
        //printf("%d %d\n", Index, Predictor_B[Index]);
    }  
    //printf("BU: %d %d\n", Index, Predictor_B[Index]);
    return Predictor_B[Index];      
}

uint32_t calculate_gshare (u_int32_t Index, uint32_t Idx_shift_G, char outcome, uint32_t Predictor_G[], unsigned long int addr, uint32_t temp, uint32_t BHR_Idx, uint32_t diff)
{
    //Predictor_G[Index] = BHR[Index][BHR_val];
    //printf("GP: %d %d\n", Index, Predictor_G[Index]);
    //printf("Before %d %d %d %d\n", Index, temp, BHR_val, Predictor_G[Index]);
    int shift = BHR_Idx - 1;
    if (outcome == 't' && Predictor_G[Index] < 3)
    {
        //printf("Before %d %d %d\n", Index, BHR_val, Predictor_G[Index]);

        Predictor_G[Index] += 1;
        //BHR[Index][BHR_val] = Predictor_G[Index];
        BHR_val = BHR_val >> 1;
        BHR_val = (Shifter(shift)+1) | BHR_val;
        //printf("After %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //printf("%d %d\n", Index, Predictor_G[Index]);
                        
    }
    else if (outcome == 't' && Predictor_G[Index] == 3)
    {
        //printf("Before %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //BHR[Index][BHR_val] = Predictor_G[Index];
        BHR_val = BHR_val >> 1;
        BHR_val = (Shifter(shift)+1) | BHR_val;                
        //printf("After %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //printf("%d %d\n", Index, Predictor_G[Index]);
    }
    else if (outcome == 'n' && Predictor_G[Index] > 0)
    {
        //printf("Before %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        Predictor_G[Index] -= 1;
        //BHR[Index][BHR_val] = Predictor_G[Index];
        BHR_val = BHR_val >> 1;
        //printf("After %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //printf("%d %d\n", Index, Predictor_G[Index]);
    }
    else if (outcome == 'n' && Predictor_G[Index] == 0)
    {
        //printf("Before %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //BHR[Index][BHR_val] = Predictor_G[Index];
        BHR_val = BHR_val >> 1;
        //printf("After %d %d %d\n", Index, BHR_val, Predictor_G[Index]);
        //printf("%d %d\n", Index, Predictor_G[Index]);
    }
    //temp = BHR_val << diff; 
    //printf("Val %d %d %d %d %d\n", Index, temp, BHR_val, Predictor_G[Index], BHR[Index][BHR_val]);
    //printf("After %d %d %d %d\n", Index, temp, BHR_val, Predictor_G[Index]);
    //printf("GU: %d %d\n", Index, Predictor_G[Index]);
    return Predictor_G[Index];       
}


int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
        /*uint32_t Idx_B = params.M2;
        uint32_t Cap_B = pow (2,Idx_B);   

        uint32_t Predictor_B[Cap_B];

        for (uint32_t i = 0; i < Cap_B; i++)
        {
            Predictor_B[i] = 2;
        }*/
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
        /*uint32_t Idx_G = params.M1;
        uint32_t Cap_G = pow (2,Idx_G);
        uint32_t Predictor_G[Cap_G];
        for (uint32_t i = 0; i < Cap_G; i++)
        {
            Predictor_G[i] = 2;
        } */

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
        /*uint32_t Idx_B = params.M2;
        uint32_t Idx_G = params.M1;
        uint32_t Idx_H = params.K;            
        uint32_t Cap_B = pow (2,Idx_B);
        uint32_t Cap_G = pow (2,Idx_G);
        uint32_t Cap_H = pow (2,Idx_H);  
        uint32_t Predictor_B[Cap_B];
        uint32_t Predictor_G[Cap_G];
        uint32_t Predictor_H[Cap_H];
        for (uint32_t i = 0; i < Cap_B; i++)
        {
            Predictor_B[i] = 2;
        }

        for (uint32_t i = 0; i < Cap_G; i++)
        {
            Predictor_G[i] = 2;
        }

        for (uint32_t i = 0; i < Cap_H; i++)
        {
            Predictor_H[i] = 1;
        } */

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    uint32_t Idx_B = params.M2;
    uint32_t Idx_G = params.M1;
    uint32_t Idx_H = params.K;            
    uint32_t Cap_B = pow (2,Idx_B);
    uint32_t Cap_G = pow (2,Idx_G);
    uint32_t Cap_H = pow (2,Idx_H);  
    uint32_t Predictor_B[Cap_B];
    uint32_t Predictor_G[Cap_G];
    uint32_t Predictor_H[Cap_H];
    uint32_t BHR_Idx =  params.N;    
    uint32_t diff = Idx_G - BHR_Idx;
    uint32_t temp;
    //uint32_t shift = BHR_Idx - 1;
    for (uint32_t i = 0; i < Cap_B; i++)
    {
        Predictor_B[i] = 2;
    }

    for (uint32_t i = 0; i < Cap_G; i++)
    {
        Predictor_G[i] = 2;
    }

    for (uint32_t i = 0; i < Cap_H; i++)
    {
        Predictor_H[i] = 1;
    }     

    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        prediction += 1;
        outcome = str[0];
        /*if (outcome == 't')
            printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
        else if (outcome == 'n')
            printf("%lx %s\n", addr, "n"); */         // Print and test if file is read correctly

        //////////////////////////////////////////////////////////
        //printf("Value is %d", Index);
        if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
        {
            uint32_t result_B = 0;
            uint32_t Idx_shift_B = Shifter(params.M2);
            uint32_t Index = (addr >> 2) & Idx_shift_B;
            char B_temp;
            if(Predictor_B[Index] == 0 || Predictor_B[Index] == 1)
            {
                B_temp = 'n';
            }
            else if(Predictor_B[Index] == 2 || Predictor_B[Index] == 3)
            {
                B_temp = 't';
            }
            if(B_temp != outcome)
            {
                misprediction += 1;
            }
            result_B = calculate_bimodal (Index, Idx_shift_B, outcome, Predictor_B, addr);
        }
        else if(strcmp(params.bp_name, "gshare") == 0)              // Gshare
        {
            uint32_t Idx_shift_G = Shifter(params.M1);
            temp = BHR_val << diff;
            //printf("Gshare start: %d %d %d\n", BHR_val, diff, temp);
            //uint32_t BHR_Idx_shift = Shifter(params.M1);   
            //uint32_t BHR_Idx = 0;
            uint32_t Index = (addr >> 2) & Idx_shift_G;
            uint32_t result_G = 0;
            if(BHR_Idx != 0)
                Index = Index ^ temp;
            char G_temp;
            if(Predictor_G[Index] == 0 || Predictor_G[Index] == 1)
            {
                G_temp = 'n';
            }
            else if(Predictor_G[Index] == 2 || Predictor_G[Index] == 3)
            {
                G_temp = 't';
            }
            if(G_temp != outcome)
            {
                misprediction += 1;
            } 
            if(BHR_Idx != 0)
            {
                result_G = calculate_gshare (Index, Idx_shift_G, outcome, Predictor_G, addr, temp, BHR_Idx, diff);
            }
            else
            {
                result_G = calculate_bimodal (Index, Idx_shift_G, outcome, Predictor_G, addr);
            }          
            //BHR[Index][BHR_val] = result_G;
            //BHR_val = Index >> diff;
 /*           for (uint32_t i = 0; i < Cap; i++)
            {
                for (uint32_t j = 0; j < BHR_Idx_Pow; j++)
                {
                    BHR[i][j] = 2;
                    printf("Here %d %d %d %d %d\n", i, j, BHR[i][j],Cap, BHR_Idx);
                }
            //Predictor[i].val = 2;
            }*/
            //printf("Val %d %d %d %d %d\n", Index, temp, BHR_val, Predictor[Index], BHR[Index][BHR_val]); 
            //printf("Gshare end: %d %d %d\n", BHR_val, diff, temp);          
        }
        else if(strcmp(params.bp_name, "hybrid") == 0)              // Hybrid
        {
            //uint32_t BHR_Idx =  params.N;
            uint32_t Idx_shift_B = Shifter(params.M2);
            uint32_t Idx_shift_G = Shifter(params.M1);
            uint32_t Idx_shift_H = Shifter(params.K);      
            //uint32_t BHR_val = 0;
            //uint32_t BHR_Idx = 0;
            //uint32_t diff = Idx_G - BHR_Idx;
            temp = BHR_val << diff;
            uint32_t Index_B = (addr >> 2) & Idx_shift_B;
            uint32_t Index_G = (addr >> 2) & Idx_shift_G;
            uint32_t Index_H = (addr >> 2) & Idx_shift_H;
            uint32_t result_B = 0, result_G = 0, result_H = 0;
            char B_res, G_res;
            result_B = Predictor_B[Index_B];
            if(BHR_Idx != 0)
                Index_G = Index_G ^ temp;
            result_G = Predictor_G[Index_G];
            if(result_B == 0 || result_B == 1)
            {
                B_res = 'n';
            }
            else if(result_B == 2 || result_B == 3)
            {
                B_res = 't';
            }

            if(result_G == 0 || result_G == 1)
            {
                G_res = 'n';
            }
            else if(result_G == 2 || result_G == 3)
            {
                G_res = 't';
            }            

            if(Predictor_H[Index_H] >= 2)
            {
                if(BHR_Idx != 0)
                {
                    result_H = calculate_gshare (Index_G, Idx_shift_G, outcome, Predictor_G, addr, temp, BHR_Idx, diff);
                    if(G_res != outcome)
                        misprediction += 1;
                }
                else
                {
                    result_H = calculate_bimodal (Index_G, Idx_shift_G, outcome, Predictor_G, addr);
                    if(G_res != outcome)
                        misprediction += 1;
                } 
            }
            else
            {
                result_H = calculate_bimodal (Index_B, Idx_shift_B, outcome, Predictor_B, addr);
                if(B_res != outcome)
                        misprediction += 1; 

                if (outcome == 't')
                {
                    //BHR[Index_G][BHR_val] = Predictor_G[Index_G];
                    BHR_val = BHR_val >> 1;
                    BHR_val = (Shifter(BHR_Idx - 1)+1) | BHR_val;                
                }
                else if (outcome == 'n')
                {
                    //BHR[Index_G][BHR_val] = Predictor_G[Index_G];
                    BHR_val = BHR_val >> 1;
                }

            }

            if((B_res != outcome) && (G_res == outcome))
            {
                if(Predictor_H[Index_H] < 3)
                {
                    Predictor_H[Index_H] += 1;
                }
            }
            else if((B_res == outcome) && (G_res != outcome))
            {
                if(Predictor_H[Index_H] > 0)
                {
                    Predictor_H[Index_H] -= 1;
                }
            }
            //printf("CP: %d %d\n", Index_H, Predictor_H[Index_H]);

            //result_B = calculate_bimodal (Index_B, Idx_shift_B, outcome, Predictor_B, addr);
            //result_G = calculate_gshare (Index_G, Idx_shift_G, outcome, Predictor_G, addr, temp, BHR_Idx, BHR_val, diff,BHR);
        }
        //////////////////////////////////////////////////////////

    }
    cout<<"OUTPUT"<<endl;
    cout<<"number of predictions:    "<<prediction<<endl;
    cout<<"number of mispredictions: "<<misprediction<<endl;
    double misprediction_rate = 100*(double(misprediction)/double(prediction));
    cout<<fixed<<setprecision(2) <<"misprediction rate:       "<<misprediction_rate<<'%'<<endl;  

    if(strcmp(params.bp_name, "bimodal") == 0)
        {   
            cout<<"FINAL BIMODAL CONTENTS"<<endl;
            for(uint32_t i = 0; i < Cap_B; i++)
            {
                cout<<i<<std::setw(6)<<Predictor_B[i]<<endl;
            }
        }
    else if(strcmp(params.bp_name, "gshare") == 0)
        {
            cout<<"FINAL GSHARE CONTENTS"<<endl;
            for(uint32_t i = 0; i < Cap_G; i++)
            {
                cout<<i<<std::setw(6)<<Predictor_G[i]<<endl;
            }
        }
    else if(strcmp(params.bp_name, "hybrid") == 0)
        { 
            cout<<"FINAL CHOOSER CONTENTS"<<endl;
            for(uint32_t i = 0; i < Cap_H; i++)
            {
                cout<<i<<std::setw(6)<<Predictor_H[i]<<endl;
            }
            cout<<"FINAL GSHARE CONTENTS"<<endl;
            for(uint32_t i = 0; i < Cap_G; i++)
            {
                cout<<i<<std::setw(6)<<Predictor_G[i]<<endl;
            }
            cout<<"FINAL BIMODAL CONTENTS"<<endl;
            for(uint32_t i = 0; i < Cap_B; i++)
            {
                cout<<i<<std::setw(6)<<Predictor_B[i]<<endl;
            }
        }

    return 0;
}
