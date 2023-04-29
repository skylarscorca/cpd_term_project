#include <cstring>
#include <iostream>
#include <stack>
using namespace std;

int main(){
    stack<char> s;
    char oneline[100];
    cout << "Please enter a string (\"quit\" to exit)\n> ";
    while(cin.getline(oneline, 100)){
        if(strcmp(oneline, "quit") == 0) break;
        string forwards = "", backwards = "";
        for(int i = 0; i < strlen(oneline); ++i){
            if(isalpha(oneline[i])){
                forwards += tolower(oneline[i]);
                s.push(tolower(oneline[i]));
            }
        }
        while(!s.empty()){
            backwards += s.top();
            s.pop();
        }
        cout << "\n\"" << oneline << "\" ";
        if(forwards != backwards) cout << " is NOT ";
        else cout << " IS ";
        cout << "a palindrome." << endl;
        cout << "\nPlease enter a string (\"quit\" to exit)\n> ";
    }
}
