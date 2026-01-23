#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime> 
#include <unistd.h>
#include <termios.h>

using namespace std;
const string PATH_CSV = "./database_weed.csv";
const string PATH_LOG = "./registro_vendite.txt";

// Palette Retro-Minimal
const string RESET  = "\033[0m";
const string AMBER  = "\033[38;5;214m"; // Arancione Fosforo per estetica
const string GREEN  = "\033[1;32m";    // Verde Profitto
const string RED    = "\033[1;31m";    // Rosso Perdita
const string DIM    = "\033[2m";       // Testo secondario

struct Carico {
    string fornitore;
    double grammiIniziali;
    double grammiRimanenti;
    double costoSostenuto; 
    double prezzoVenditaAlG; 
    double utileAttuale;   
    string dataCarico;

    double getGrammiVenduti() const { return grammiIniziali - grammiRimanenti; }
    double getGrammiPerPareggio() const {
        if (utileAttuale >= 0) return 0.0;
        return (-utileAttuale) / prezzoVenditaAlG;
    }
};

struct VenditaTemporanea {
    string data;
    string cliente;
    double soldi;
};

vector<Carico> magazzino;
vector<VenditaTemporanea> bufferVendite;

string dataAttuale() {
    time_t t = time(0);
    struct tm * now = localtime(&t);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", now);
    return string(buf);
}

void mostraLogo() {
    system("clear");
    cout << AMBER << " ..........................................................." << endl;
    cout << "   _ _ _ _____ _____ ___     _____ _____ _____ _____ _____ " << endl;
    cout << "  | | | |   __|   __|   \\   |     |  _  |   | |  _  |   __|" << endl;
    cout << "  | | | |   __|   __|  |  |  | | | |     | | | |     |  |  |" << endl;
    cout << "  |_____|_____|_____|___/   |_|_|_|__|__|_|___|__|__|_____|" << endl;
    cout << " ........................................... system v1.12 .." << RESET << endl;
}

bool checkEscape() {
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    int ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    if (ch == 27) {
        return true;
    }
    ungetc(ch, stdin);
    return false;
}

void salvaTutto() {
    ofstream file(PATH_CSV);
    file << "Fornitore,GrammiIniziali,Rimanenti,CostoSostenuto,PrezzoVenditaAlG,UtileAttuale,DataCarico\n";
    for (const auto& c : magazzino) {
        file << c.fornitore << "," << c.grammiIniziali << "," << c.grammiRimanenti << "," 
             << c.costoSostenuto << "," << c.prezzoVenditaAlG << "," << c.utileAttuale << "," << c.dataCarico << "\n";
    }
    file.close();

    if (!bufferVendite.empty()) {
        ofstream logFile(PATH_LOG, ios::app);
        for (const auto& v : bufferVendite) {
            logFile << "[" << v.data << "] USER: " << v.cliente << " | CREDIT: " << fixed << setprecision(2) << v.soldi << " EUR" << endl;
        }
        logFile.close();
    }
}

void caricaDati() {
    ifstream file(PATH_CSV);
    if (!file.is_open()) return;
    string riga;
    getline(file, riga); 
    while (getline(file, riga)) {
        stringstream ss(riga);
        Carico c; string temp;
        getline(ss, c.fornitore, ',');
        getline(ss, temp, ','); c.grammiIniziali = stod(temp);
        getline(ss, temp, ','); c.grammiRimanenti = stod(temp);
        getline(ss, temp, ','); c.costoSostenuto = stod(temp);
        getline(ss, temp, ','); c.prezzoVenditaAlG = stod(temp);
        getline(ss, temp, ','); c.utileAttuale = stod(temp);
        getline(ss, c.dataCarico, ',');
        magazzino.push_back(c);
    }
    file.close();
}

int main() {
    caricaDati();
    int scelta;
    do {
        mostraLogo();
        cout << "\n " << AMBER << "[01]" << RESET << " New Input Sequence" << endl;
        cout << " " << AMBER << "[02]" << RESET << " Execute Discharge" << endl;
        cout << " " << AMBER << "[03]" << RESET << " Access Ledger File" << endl;
        cout << " " << AMBER << "[04]" << RESET << " Read Chrono Log" << endl;
        cout << " " << AMBER << "[05]" << RESET << " Shutdown System" << endl;
        cout << DIM << "\n root@hawkins_lab > " << RESET;
        cin >> scelta;

        if (scelta == 1) {
            mostraLogo();
            Carico c;
            cout << AMBER << ">> INCOMING_SHIPMENT_DATA" << RESET << "\n\n";
            cout << " Source: "; cin >> c.fornitore;
            cout << " Mass (g): "; cin >> c.grammiIniziali;
            cout << " Cost (tot): "; cin >> c.costoSostenuto;
            cout << " Retail/g: "; cin >> c.prezzoVenditaAlG;
            cout << " Date: "; cin >> c.dataCarico;
            c.grammiRimanenti = c.grammiIniziali;
            c.utileAttuale = -c.costoSostenuto; 
            magazzino.push_back(c);
            cout << "\n " << AMBER << ">> BATCH_SYNCED." << RESET;
            cin.ignore(); cin.get();
        } 
        else if (scelta == 2) {
            mostraLogo();
            if (magazzino.empty()) continue;
            cout << AMBER << ">> DISCHARGE_SEQUENCE" << RESET << "\n\n";
            cout << " Entry ID: "; int idx; cin >> idx;
            if (idx >= 0 && idx < (int)magazzino.size()) {
                double soldi; string cliente;
                cout << " Client: "; cin >> cliente;
                cout << " Revenue: "; cin >> soldi;
                double g = soldi / magazzino[idx].prezzoVenditaAlG; 
                if (g <= magazzino[idx].grammiRimanenti) {
                    magazzino[idx].grammiRimanenti -= g; 
                    magazzino[idx].utileAttuale += soldi;      
                    bufferVendite.push_back({dataAttuale(), cliente, soldi});
                    cout << "\n " << AMBER << ">> TRANSACTION_STAGED." << RESET;
                }
            }
            cin.ignore(); cin.get();
        }
        else if (scelta == 3) {
            system("clear");
            cout << AMBER << ">> LEDGER_DATABASE_VIEW" << RESET << endl;
            cout << DIM << " ........................................................................." << RESET << endl;
            cout << left << setw(4) << "ID" << setw(15) << "SOURCE" << setw(12) << "DATE"
                 << setw(10) << "SOLD" << setw(10) << "LEFT" << setw(12) << "PROFIT" << setw(10) << "B-EVEN" << endl;
            cout << DIM << " ........................................................................." << RESET << endl;
            
            for (int i = 0; i < (int)magazzino.size(); i++) {
                string pCol;
                if (magazzino[i].utileAttuale < 0) pCol = RED;
                else if (magazzino[i].utileAttuale == 0) pCol = AMBER; // Ambra per il pareggio
                else pCol = GREEN;

                cout << " " << left << setw(4) << i << setw(15) << magazzino[i].fornitore << setw(12) << magazzino[i].dataCarico
                     << setw(10) << fixed << setprecision(2) << magazzino[i].getGrammiVenduti()
                     << setw(10) << magazzino[i].grammiRimanenti;
                
                cout << pCol << setw(12) << magazzino[i].utileAttuale << RESET;
                
                cout << setw(10) << magazzino[i].getGrammiPerPareggio() << endl;
            }
            cout << DIM << " ........................................................................." << RESET << endl;
            cout << DIM << " [Enter] to return" << RESET;
            cin.ignore(); cin.get();
        }
        else if (scelta == 4) {
            mostraLogo();
            cout << AMBER << ">> LOADING_CHRONO_LOG_STREAM..." << RESET << "\n" << endl;
            // 'cat' stampa il contenuto del file direttamente nel terminale
            string comando = "cat " + PATH_LOG; 
            system(comando.c_str()); 
            
            cout << DIM << "\n..........................................." << RESET;
            cout << DIM << "\n[Enter] to return to system" << RESET;
            cin.ignore(); cin.get();
        }
    } while (scelta != 5);

    mostraLogo();
    cout << AMBER << " WRITING_TO_DISK..." << RESET << endl;
    salvaTutto();
    usleep(800000);
    cout << DIM << " SESSION_TERMINATED." << RESET << endl;
    return 0;
}