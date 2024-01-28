#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

static char field[10][7];
int MAX_LENGTH = 100;
char* list[20]; // a list to keep max 20 arguments

struct mac {
char mname[8]; //macro name
char param[10][5]; //max. 10 parameters, each parameter max. 4 characters
char macro[256]; //macro body
};

struct mac buffer[10]; // memory buffer for 10 macro definitions

int m_count;

struct pt {
    char mname[8]; //macro name
    int nparams; //number of parameters
    char dummy[10][5]; //max. 10 parameters, each parameter max. 4 characters 
    char actual[10][5];
};
struct pt PT;
int read(char* filename){
    for(int i = 0; i < 10; i++){
        memset(buffer[i].mname, 0, sizeof(buffer[i].mname));
        memset(buffer[i].param, 0, sizeof(buffer[i].param));
        memset(buffer[i].macro, 0, sizeof(buffer[i].macro));
    }
    int m_count = 0;
    
    FILE* file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("Error opening the file.\n");
        return 0;
    }
    
    char line[MAX_LENGTH];
    const char delimeter[] = " ,:";
    
    while (fgets(line, MAX_LENGTH, file) != NULL) {
        // Tokenize the line by spaces
        char* token;
        char* rest = line;
        
        if (strncmp(line, "#ENDM", 5) != 0 && line[0] == '#' && strstr(line, ":") != NULL){
            // Macro name
            token = strtok_r(rest, delimeter, &rest);
            if (token != NULL) {
                strcpy(buffer[m_count].mname, token+1);
            }
            token = strtok_r(rest, delimeter, &rest);
            
            // Parameters
            int param_count = 0;
            while ((token = strtok_r(rest, delimeter, &rest)) != NULL && param_count < 10) {
                if(*(token+strlen(token)-1) == '\n') *(token+strlen(token)-1) = '\0';
                strncpy((buffer)[m_count].param[param_count], token, sizeof(buffer)[m_count].param[param_count]);
                param_count++;
            }
        }
        else if(strncmp(line, "#ENDM", 5) == 0){
            //end of macro
            m_count++;
        } else {
            strcat(buffer[m_count].macro, line);
        }
    }
    fclose(file);
    return m_count;
}

void createPT(){
    memset(PT.mname, 0, sizeof(PT.mname));
    PT.nparams = 0;
    memset(PT.dummy, 0, sizeof(PT.dummy));
    memset(PT.actual, 0, sizeof(PT.actual));
    int numberOfParams=0;
    if(field[0][0]=='#'){ //starts with if
        bool added = false;
        strcpy(PT.mname,field[3]);
        for(int i=4;i<10;i++){
            for(int j=0; j<sizeof(field[i]) && field[i][j]!='\0';j++){
                PT.actual[i-4][j]=field[i][j];
                added=true;
            }
            if(added) numberOfParams+=1;
            added=false;
       }
    
    }else{ //starts with macro call
        bool added=false;
        strcpy(PT.mname,field[0]);
        
        int j=1;
        for(;j<10;j++){
            for( int k=0;k<sizeof(field[j]) && field[j][k]!='\0';k++){
                PT.actual[j-1][k]=field[j][k];
                added=true;
            }  
            if(added)numberOfParams+=1;
            added=false;
        }
    }
    bool found=false;
    for(int i=0; i<m_count;i++){ // checking for macro name in all of the macros
        if(!found){
            for(int j=0; j<sizeof(PT.mname);j++){
                if(PT.mname[j]==buffer[i].mname[j]){
                    found=true;
                }
                else{
                    found = false;
                    break;
                }
            }
            if(found){
                for(int j=0;j<numberOfParams;j++){ // creating the dummy parameters
                    strcpy(PT.dummy[j], buffer[i].param[j]);
                }
                break;
            }
        }
    }
    PT.nparams=numberOfParams;
}

void expand(){
    FILE* fPtr = fopen("f1.asm", "a"); // open file for appending
    createPT(); // creating the Parameter Table
    
    for(int i = 0; i<m_count;i++){// checking for macro name in all of the macros
        bool is_same_mname = false;
        for(int s = 0; s<sizeof(PT.mname);s++){
            if(PT.mname[s] == buffer[i].mname[s]){
                is_same_mname = true;
                continue;
            }
            is_same_mname = false;
            break;
        }
        if(is_same_mname){ // if macro name is found start iterating through macrobody
            int j = 0;
            int size_of_dummy = 0;
            int which_dummy = 0;
            while((j<sizeof(buffer[i].macro)) && (buffer[i].macro[j]!='\0')){// iterate through macro
                bool is_dummy = false;
                if(buffer[i].macro[j-1]==' '){
                    for(int k=0; k<PT.nparams; k++){  // search if there is a valid dummy parameter in macrobody
                        if(buffer[i].macro[j] == PT.dummy[k][0]){
                            for(int p=0;(p<sizeof(PT.dummy[k]) && PT.dummy[k][p]!='\0');p++){
                                if(PT.dummy[k][p] == buffer[i].macro[j+size_of_dummy]){
                                    is_dummy = true;
                                    size_of_dummy ++;
                                }
                                else{
                                    is_dummy = false;
                                    size_of_dummy = 0;
                                    break;
                                }
                            }
                            if(buffer[i].macro[j+size_of_dummy] != '\n'){
                                is_dummy = false;
                            }
                        }
                        if(is_dummy){
                            which_dummy = k;
                            break;
                        }
                    }
                }
                if(is_dummy){ // if a dummy parameter is found
                    for(int a = 0; ((a<4) && (PT.actual[which_dummy][a]!='\0')); a++){// changing dummy parameters with the actual ones and then appending to the file
                        fprintf(fPtr,"%c",PT.actual[which_dummy][a]);
                    }
                    j = j + size_of_dummy;
                    size_of_dummy = 0;
                }
                else{
                    fprintf(fPtr,"%c",buffer[i].macro[j]); // append normal chars in the macro to the file 
                    j+=1;
                }
            }
        }
    }
    fclose(fPtr); // closing the file
}

void is_macro() { 
        //read a field
        //checks if there is a macro call #if
        if((field[0][0] == '#') && (field[0][1] == 'i') && (field[0][2] == 'f') ) { 
            
            int num=0;
            for(int i=1; i<7; i++){
                if(field[1][i] != '\0'){
                    num = num * 10 + (field[1][i] -'0'); //find which index is the argument in the list
                }
            }
            
            //check if  #if's condition is true call expand function
            bool is_same_as_argument = false;
            for(int c=0;c<(sizeof(list[num])/8);c++){
                if(((list[num][c]==field[2][c]) && (field[2][c]!='\0')&&(list[num][c]!='\0'))){
                    is_same_as_argument = true;
                }
                else{
                    is_same_as_argument = false;
                    break;
                }
            }
            if(is_same_as_argument){
                expand(); //#if is found an its condition is true
            }
        } 
        else{ 
            bool isTrue = false;
            //checks if field's first element is equal to any macro name in the buffer
            for(int x=0; x < m_count; x++){ 
                // 7 is the field second dimensions length
                for(int h=0; h<7; h++){
                    //check the conditions until a null value is found
                    if(field[0][h] != '\0'){
                        if( field[0][h] == buffer[x].mname[h]){
                            isTrue = true;
                        }
                        else{
                            isTrue = false;
                            break;
                        }
                    }
                }
                if(isTrue) break;
            }
            if(isTrue) {
                expand(); // a macro name is found among the buffer names
            }
            //if there is no macro call in that line, line is appended to f1.asm file
            if(isTrue == false){
                //open f1.asm to append
                FILE *fp;   
                fp = fopen("f1.asm", "a+");
                bool isNull = false;
                int i = 0;
                while(i < 10){
                    int k = 0;
                    while(k<7){
                        if(field[i][k] != '\0'){
                            fprintf(fp,"%c", field[i][k]);
                            isNull= true;
                            k++;
                        }
                        else {
                            k=7;
                        }
                    }
                    i++;
                    if(isNull){
                        fprintf(fp," ");  
                    } 
                }
                if(isNull) {
                    fprintf(fp,"\n" );  
                }
                fclose(fp);
            }
        }
    }
    
void parse(char* filename){
    
    //assign delimiter characters and open the input file
    const char delim[] = " ,(=')";
    FILE* fPtr = fopen(filename, "r");
    
    char line[MAX_LENGTH];
    bool endOfMacroDefinition = false;
    int endmCount = 0;
    
    //loop through the file until you come across the end of file (feof)
    while(!feof(fPtr)){
            
        //fill the array with zeros. you can check if a field element is occupied by if(strlen(fields[i][j]))
        memset(field, 0, sizeof(field));
        
        //get line
        fgets(line, MAX_LENGTH, fPtr);
        
        //do not start parsing until you see all macro definitions
        if(strstr(line, "#ENDM") != NULL){
            endmCount++;
            if(endmCount == m_count){
                endOfMacroDefinition = 1;
                continue;
            }
        }
        if(!endOfMacroDefinition){
            continue;
        }
        
        //get the first field discretely to keep initial spaces
        if(line[0] != '#' || !strncmp(line, "#if", 3)){
            int i = 0;
            while(i < strlen(line) && line[i] == 32){
                field[0][i] = line[i];
                i++;
            }
            while(i < strlen(line) && line[i] != ' ' && line[i] != 0 && line[i] != '\n'){
                field[0][i] = line[i];
                i++;
            }
        }
        else{ //if the token is a macro ("#macroname"), the # is removed
            int i = 1;
            while(i < strlen(line) && line[i] != ' ' && line[i] != 0 && line[i] != '\n'){
                field[0][i-1] = line[i];
                i++;
            }
        }
        
        char* token = strtok(line, delim);
        int fieldCount = 1;
        token = strtok(NULL, delim);
        
        while (token != NULL && fieldCount < 10)  {
            strncpy(field[fieldCount], token, 6);
            
            //inserts null character at the end of field in case of overflowing
            field[fieldCount][6] = '\0';
            //inserts null characters instead of new line
            for(int i = 0; i < strlen(field[fieldCount]); i++){
                if(field[fieldCount][i] == '\n'){
                    field[fieldCount][strlen(field[fieldCount])-1] = '\0';
                }
            }
            fieldCount++;
            
            //gets next token for next iteration
            token = strtok (NULL, delim);
        }
        
        is_macro();
        
        // to show coontents of current line
        if(true) {
            if(strlen(field[0])){
                for(int j = 0; j < 10; j++){
                    if(strlen(field[j]))
                        printf("%s ", field[j]);
                }
            }
            printf("\n");
        }
    }
    fclose(fPtr);
}

int main(int argc, char *argv[])
{
    char* filename = argv[1];
    m_count = read(filename);
    for (int i = 0; i < argc; i++) {
        list[i] = argv[i];
    }
    parse(filename);
    return 0;
}