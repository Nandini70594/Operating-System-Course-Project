#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
using namespace std;

char M[300][4], IR[4], R[4];
bool C, used[30], finishJob;
int IC, PTR, SI, PI, TI;

struct PCB {
    string jobId;
    int TTL, TLL, TTC, LLC;
} pcb;

ifstream fin("myinput.txt");
ofstream fout("myoutput.txt");
string line;

bool digit(char c) {
    return c >= '0' && c <= '9';
}

int operand() {
    if (!digit(IR[2]) || !digit(IR[3])) {
        PI = 2;
        return -1;
    }
    return (IR[2] - '0') * 10 + (IR[3] - '0');
}

void init() {
    memset(M, ' ', sizeof(M));
    memset(IR, ' ', sizeof(IR));
    memset(R, ' ', sizeof(R));
    memset(used, false, sizeof(used));

    IC = PTR = SI = PI = TI = 0;
    C = false;
    finishJob = false;

    pcb.TTC = 0;
    pcb.LLC = 0;
}

int allocateFrame() {
    while (true) {
        int f = rand() % 30;
        if (!used[f]) {
            used[f] = true;
            return f;
        }
    }
}

void createPageTable() {
    int f = allocateFrame();
    PTR = f * 10;

    for (int i = PTR; i < PTR + 10; i++) {
        M[i][0] = '*';
        M[i][1] = '*';
        M[i][2] = '*';
        M[i][3] = '*';
    }
}

void setPTE(int page, int frame) {
    int pte = PTR + page;
    M[pte][0] = '0';
    M[pte][1] = '0';
    M[pte][2] = frame / 10 + '0';
    M[pte][3] = frame % 10 + '0';
}

void allocatePage(int VA) {
    int page = VA / 10;
    int frame = allocateFrame();

    setPTE(page, frame);

    for (int i = frame * 10; i < frame * 10 + 10; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = ' ';
}

int addressMap(int VA) {
    if (VA < 0 || VA > 99) {
        PI = 2;
        return -1;
    }
    int page = VA / 10;
    int offset = VA % 10;
    int pte = PTR + page;
    if (M[pte][0] == '*') {
        PI = 3;
        return -1;
    }
    int frame = (M[pte][2] - '0') * 10 + (M[pte][3] - '0');
    return frame * 10 + offset;
}

void terminate(int em) {
    if (finishJob) return;
    finishJob = true;
    fout << "\n\nJOB ID: " << pcb.jobId << "\n";
    switch (em) {
        case 0: fout << "NO ERROR\n"; break;
        case 1: fout << "OUT OF DATA\n"; break;
        case 2: fout << "LINE LIMIT EXCEEDED\n"; break;
        case 3: fout << "TIME LIMIT EXCEEDED\n"; break;
        case 4: fout << "OPCODE ERROR\n"; break;
        case 5: fout << "OPERAND ERROR\n"; break;
        case 6: fout << "INVALID PAGE FAULT\n"; break;
    }

    fout << "TTC=" << pcb.TTC << " LLC=" << pcb.LLC << "\n";
}

void executeUserProgram() {
    IC = 0;
    while (!finishJob) {
        int RA = addressMap(IC);
        if (PI != 0) { terminate(5); break; }
        for (int i = 0; i < 4; i++)
            IR[i] = M[RA][i];
        IC++;
        if (IR[0] == 'H') {
            terminate(0);
            break;
        }

        int VA = -1;
        bool needsVA = !(IR[0]=='H' || (IR[0]=='B' && IR[1]=='T'));
        if (needsVA) {
            VA = operand();
            if (PI != 0) { terminate(5); break; }
        }

        if (IR[0]=='G' && IR[1]=='D') {
            getline(fin, line);
            int k = 0;
            for (int i = 0; i < 10; i++) {
                int r = addressMap(VA + i);
                if (PI == 3) {
                    PI = 0;
                    allocatePage(VA + i);
                    r = addressMap(VA + i);
                }
                for (int j = 0; j < 4; j++)
                    M[r][j] = (k < (int)line.size()) ? line[k++] : ' ';
            }
        }

        else if (IR[0]=='P' && IR[1]=='D') {
            pcb.LLC++;
            if (pcb.LLC > pcb.TLL) { terminate(2); break; }

            for (int i = 0; i < 10; i++) {
                int r = addressMap(VA + i);
                for (int j = 0; j < 4; j++)
                    fout << M[r][j];
            }
            fout << "\n";
        }

        else if (IR[0]=='L' && IR[1]=='R') {
            int r = addressMap(VA);
            for (int i = 0; i < 4; i++) R[i] = M[r][i];
        }

        else if (IR[0]=='S' && IR[1]=='R') {
            int r = addressMap(VA);
            for (int i = 0; i < 4; i++) M[r][i] = R[i];
        }

        else if (IR[0]=='C' && IR[1]=='R') {
            int r = addressMap(VA);
            C = true;
            for (int i = 0; i < 4; i++)
                if (R[i] != M[r][i]) C = false;
        }

        else if (IR[0]=='B' && IR[1]=='T') {
            if (C) {
                if (digit(IR[2]) && digit(IR[3]))
                    IC = (IR[2]-'0')*10 + (IR[3]-'0');
                else {
                    PI = 2;
                    terminate(5);
                    break;
                }
            }
        }

        else {
            terminate(4);
            break;
        }
        pcb.TTC++;

        if (pcb.TTC > pcb.TTL) {
            terminate(3);
            break;
        }
    }
}

void load() {
    int VA = 0;
    while (getline(fin, line)) {
        if (line.substr(0,4) == "$AMJ") {
            init();
            createPageTable();
            VA = 0;
            stringstream ss(line.substr(5));
            ss >> pcb.jobId >> pcb.TTL >> pcb.TLL;
        }
        else if (line.substr(0,4) == "$DTA") {
            executeUserProgram();
        }
        else if (line.substr(0,4) == "$END") {
            continue;
        }
        else {
            int k = 0;
            while (k < (int)line.size()) {
                char word[4] = {' ',' ',' ',' '};
                for (int i = 0; i < 4 && k < (int)line.size(); i++)
                    word[i] = line[k++];
                int page = VA / 10;
                if (M[PTR + page][0] == '*')
                    allocatePage(VA);
                int r = addressMap(VA);
                for (int i = 0; i < 4; i++)
                    M[r][i] = word[i];
                VA++;
            }
        }
    }
}

int main() {
    srand(10);
    if (!fin) {
        cout << "input file not found\n";
        return 0;
    }
    load();
    fin.close();
    fout.close();
    return 0;
}
