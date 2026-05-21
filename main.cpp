#include <iostream>
#include <fstream>
#include <string>
using namespace std;

char M[100][4];
char IR[4];
char R[4];
bool C = false;
int IC = 0;
int SI = 0;

string buffer;
ifstream fin("input.txt");
ofstream fout("output.txt");

int getAddress() {
    return (IR[2] - '0') * 10 + (IR[3] - '0');
}

void init() {
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = ' ';
    IC = 0;
    C = false;
    for (int i = 0; i < 4; i++)
        R[i] = ' ';
}

void read() {
    getline(fin, buffer);
    int loc = getAddress();
    int k = 0;
    for (int i = loc; i < loc + 10 && i < 100; i++) {
        for (int j = 0; j < 4; j++) {
            if (k < buffer.length())
                M[i][j] = buffer[k++];
            else
                M[i][j] = ' ';
        }
    }
}

void write() {
    int loc = getAddress();
    for (int i = loc; i < loc + 10 && i < 100; i++) {
        for (int j = 0; j < 4; j++)
            fout << M[i][j];
    }
    fout << endl;
}

void MOS() {
    if (SI == 1)
        read();
    else if (SI == 2)
        write();
}

void execute() {
    while (true) {
        for (int i = 0; i < 4; i++)
            IR[i] = M[IC][i];
        IC++;

        if (IR[0] == 'G' && IR[1] == 'D') {
            SI = 1;
            MOS();
        } else if (IR[0] == 'P' && IR[1] == 'D') {
            SI = 2;
            MOS();
        } else if (IR[0] == 'L' && IR[1] == 'R') {
            int loc = getAddress();
            for (int i = 0; i < 4; i++)
                R[i] = M[loc][i];
        } else if (IR[0] == 'S' && IR[1] == 'R') {
            int loc = getAddress();
            for (int i = 0; i < 4; i++)
                M[loc][i] = R[i];
        } else if (IR[0] == 'C' && IR[1] == 'R') {
            int loc = getAddress();
            C = true;
            for (int i = 0; i < 4; i++)
                if (R[i] != M[loc][i])
                    C = false;
        } else if (IR[0] == 'B' && IR[1] == 'T') {
            if (C)
                IC = getAddress();
        } else if (IR[0] == 'H') {
            break;
        }
    }
}

void load() {
    int m = 0;
    while (getline(fin, buffer)) {
        if (buffer.substr(0, 4) == "$AMJ") {
            init();
            m = 0;
        }
        else if (buffer.substr(0, 4) == "$DTA") {
            execute();
        }
        else if (buffer.substr(0, 4) == "$END") {
            continue;
        }
        else {
            int col = 0;
            for (char ch : buffer) {
                M[m][col++] = ch;
                if (col == 4) {
                    col = 0;
                    m++;
                }
            }
            if (col != 0)
                m++;
        }
    }
}

int main() {
    if (!fin) {
        cout << "Error: input.txt not found\n";
        return 1;
    }
    load();
    fin.close();
    fout.close();
    return 0;
}
