#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

// apt-get update && apt-get install -y nlohmann-json3-dev vim g++ net-tools

#define CAMPO 10
#define ECHOMAX 1024

using json = nlohmann::json;
using namespace std;

bool verificarMensagem(string &mensagem) {
	regex padrao("^[0-9][0-9]$");

	return regex_match(mensagem, padrao);
}

void gerarNavio(string tipo, vector<vector<int>> &posicoesNavio, vector<json> &navios) {
	json navio;
	navio["tipo"] = tipo;
	navio["posicoes"] = posicoesNavio;
	navios.push_back(navio);
}

bool verificarSobreposicao(const vector<vector<int>> &todasPosicoes, const vector<int> &novaPosicao) {
	for(const auto &pos : todasPosicoes) {
		if(pos == novaPosicao) {
			return true;
		}
	}
	return false;
}

void gerarPosicoes(int tamanho, vector<vector<int>> &todasPosicoes, string tipo, vector<json> &navios) {
	bool sobrepor;
	bool isHorizontal = rand() % 2;
	vector<vector<int>> posicoesNavio;

	if(isHorizontal) {
		while(true) {
			vector<int> posicaoInicial = {rand() % CAMPO, rand() % CAMPO};

			if(posicaoInicial[1] + tamanho > CAMPO) {
				continue;
			}

			posicoesNavio.clear();

			sobrepor = false;
			for(int i = 0; i < tamanho; ++i) {
				vector<int> posicao = {posicaoInicial[0], posicaoInicial[1] + i};
				if(verificarSobreposicao(todasPosicoes, posicao)) {
					sobrepor = true;
					break;
				}
				posicoesNavio.push_back(posicao);
			}

			if(!sobrepor) {
				gerarNavio(tipo, posicoesNavio, navios);
				for(const auto &pos : posicoesNavio) {
					todasPosicoes.push_back(pos);
				}
				break;
			}
		}
	} else {
		while(true) {
			vector<int> posicaoInicial = {rand() % CAMPO, rand() % CAMPO};

			if(posicaoInicial[0] + tamanho > CAMPO) {
				continue;
			}

			vector<vector<int>> posicoesNavio;
			sobrepor = false;
			for(int i = 0; i < tamanho; ++i) {
				vector<int> posicao = {posicaoInicial[0] + i, posicaoInicial[1]};
				if(verificarSobreposicao(todasPosicoes, posicao)) {
					sobrepor = true;
					break;
				}
				posicoesNavio.push_back(posicao);
			}

			if(!sobrepor) {
				gerarNavio(tipo, posicoesNavio, navios);
				for(const auto &pos : posicoesNavio) {
					todasPosicoes.push_back(pos);
				}
				break;
			}
		}
	}
}

void criarCampo(string nome) {
	vector<vector<string>> campoBatalha(CAMPO, vector<string>(CAMPO, "🟦"));

	json campo = campoBatalha;

	ofstream arquivoCampo(nome);

	if(arquivoCampo.is_open()) {
		arquivoCampo << campo;
	}
	arquivoCampo.close();
}

void exibirCamposPosTiro(vector<vector<int>> &todasPosicoes, json &navios, vector<int> &posTiro, bool meuTiro) {
	json campo, campoAdv;

	ifstream campoIN("campoAdv.json");
	ifstream campoIN2("campo.json");

	if(campoIN.is_open()) {
		campoIN >> campoAdv;
		campoIN.close();
	}

	if(campoIN2.is_open()) {
		campoIN2 >> campo;
		campoIN2.close();
	}

	bool acertou = false;

	if(meuTiro) {
		for(const auto &navio : navios) {
			for(const auto &pos : navio["posicoes"]) {
				if(pos == posTiro) {
					string tipoNavio = navio["tipo"].get<string>();
					if(tipoNavio == "porta-avioes")
						campoAdv[posTiro[0]][posTiro[1]] = "🛳️ ";
					else if(tipoNavio == "encouracado")
						campoAdv[posTiro[0]][posTiro[1]] = "🚢";
					else if(tipoNavio == "cruzador")
						campoAdv[posTiro[0]][posTiro[1]] = "🛥️ ";
					else if(tipoNavio == "destroier")
						campoAdv[posTiro[0]][posTiro[1]] = "🚤";
					acertou = true;
				}
			}
		}

		if(!acertou) {
			campoAdv[posTiro[0]][posTiro[1]] = "❌";
		}

	} else {
		for(const auto &navio : navios) {
			for(const auto &pos : navio["posicoes"]) {
				if(pos == posTiro) {
					string tipoNavio = navio["tipo"].get<string>();
					if(tipoNavio == "porta-avioes")
						campo[posTiro[0]][posTiro[1]] = "🛳️ ";
					else if(tipoNavio == "encouracado")
						campo[posTiro[0]][posTiro[1]] = "🚢";
					else if(tipoNavio == "cruzador")
						campo[posTiro[0]][posTiro[1]] = "🛥️ ";
					else if(tipoNavio == "destroier")
						campo[posTiro[0]][posTiro[1]] = "🚤";
					acertou = true;
				}
			}
		}

		if(!acertou) {
			campo[posTiro[0]][posTiro[1]] = "❌";
		}
	}

	for(int i = 0; i < todasPosicoes.size(); ++i) {
		if(todasPosicoes[i] == posTiro) {
			todasPosicoes.erase(todasPosicoes.begin() + i);
			break;
		}
	}

	cout << "            " << "MEU CAMPO" << "                          " << "CAMPO ADVERSÁRIO" << endl;
	cout << "   ";
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do seu campo
	}
	cout << "       "; // Espaço entre os campos
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do campo adversário
	}
	cout << endl;

	for(int i = 0; i < 10; ++i) {
		cout << " " << i << " "; // Índice de linha do seu campo
		for(int j = 0; j < 10; ++j) {
			cout << campo[i][j].get<string>() << " "; // Seu campo
		}

		cout << "    "; // Espaço entre os campos

		cout << " " << i << " "; // Índice de linha do campo adversário
		for(int j = 0; j < 10; ++j) {
			cout << campoAdv[i][j].get<string>() << " "; // Campo adversário
		}
		cout << endl;
	}

	ofstream campoOUT("campoAdv.json");
	ofstream campoOUT2("campo.json");

	if(campoOUT.is_open()) {
		campoOUT << campoAdv;
	}
	campoOUT.close();

	if(campoOUT2.is_open()) {
		campoOUT2 << campo;
	}
	campoOUT2.close();
}

void lerArquivoNavios(vector<json> &navios, string nome) {
	json naviosTemp;
	ifstream arqNavios(nome);
	arqNavios >> naviosTemp;
	arqNavios.close();

	for(const auto &navio : naviosTemp) {
		navios.push_back(navio);
	}
}

void mostrarNavios() {
	vector<json> navios;
	lerArquivoNavios(navios, "navios.json");
	
	vector<vector<string>> campo(CAMPO, vector<string>(CAMPO, "🟦"));

	for(const auto &navio : navios) {
		for(const auto &pos : navio["posicoes"]) {
			string tipoNavio = navio["tipo"].get<string>();
			if(tipoNavio == "porta-avioes")
				campo[pos[0]][pos[1]] = "🛳️ ";
			else if(tipoNavio == "encouracado")
				campo[pos[0]][pos[1]] = "🚢";
			else if(tipoNavio == "cruzador")
				campo[pos[0]][pos[1]] = "🛥️ ";
			else if(tipoNavio == "destroier")
				campo[pos[0]][pos[1]] = "🚤";
		}
	}

	cout << "\n";
	cout << "            MEUS NAVIOS" << endl;
	cout << "   ";
	for(int col = 0; col < 10; col++) {
		cout << col << "  ";
	}
	cout << endl;

	for(int i = 0; i < 10; ++i) {
		cout << " " << i << " ";
		for(int j = 0; j < 10; ++j) {
			cout << campo[i][j] << " ";
		}
		cout << endl;
	}
	cout << "\n";
}

void mostrarNaviosFimPartida(vector<json> &navios, vector<json> &naviosAdv) {

	vector<vector<string>> campo(CAMPO, vector<string>(CAMPO, "🟦"));
	vector<vector<string>> campoAdv(CAMPO, vector<string>(CAMPO, "🟦"));

	for(const auto &navio : navios) {
		for(const auto &pos : navio["posicoes"]) {
			string tipoNavio = navio["tipo"].get<string>();
			if(tipoNavio == "porta-avioes")
				campo[pos[0]][pos[1]] = "🛳️ ";
			else if(tipoNavio == "encouracado")
				campo[pos[0]][pos[1]] = "🚢";
			else if(tipoNavio == "cruzador")
				campo[pos[0]][pos[1]] = "🛥️ ";
			else if(tipoNavio == "destroier")
				campo[pos[0]][pos[1]] = "🚤";
		}
	}

	for(const auto &navio : naviosAdv) {
		for(const auto &pos : navio["posicoes"]) {
			string tipoNavio = navio["tipo"].get<string>();
			if(tipoNavio == "porta-avioes")
				campoAdv[pos[0]][pos[1]] = "🛳️ ";
			else if(tipoNavio == "encouracado")
				campoAdv[pos[0]][pos[1]] = "🚢";
			else if(tipoNavio == "cruzador")
				campoAdv[pos[0]][pos[1]] = "🛥️ ";
			else if(tipoNavio == "destroier")
				campoAdv[pos[0]][pos[1]] = "🚤";
		}
	}

	cout << "\n";
	cout << "            " << "MEUS NAVIOS" << "                        " << "NAVIOS ADVERSÁRIO" << endl;
	cout << "   ";
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do seu campo
	}
	cout << "       "; // Espaço entre os campos
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do campo adversário
	}
	cout << endl;

	for(int i = 0; i < 10; ++i) {
		cout << " " << i << " "; // Índice de linha do seu campo
		for(int j = 0; j < 10; ++j) {
			cout << campo[i][j] << " "; // Seu campo
		}

		cout << "    "; // Espaço entre os campos

		cout << " " << i << " "; // Índice de linha do campo adversário
		for(int j = 0; j < 10; ++j) {
			cout << campoAdv[i][j] << " "; // Campo adversário
		}
		cout << endl;
	}
	cout << "\n";
}

void exibirCamposInicial() {
	json campo, campoAdv;

	ifstream campoIN("campoAdv.json");
	ifstream campoIN2("campo.json");

	if(campoIN.is_open()) {
		campoIN >> campoAdv;
	}

	if(campoIN2.is_open()) {
		campoIN2 >> campo;
	}

	campoIN.close();
	campoIN2.close();

	cout << "            " << "MEU CAMPO" << "                          " << "CAMPO ADVERSÁRIO" << endl;
	cout << "   ";
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do seu campo
	}
	cout << "       "; // Espaço entre os campos
	for(int col = 0; col < 10; col++) {
		cout << col << "  "; // Cabeçalho do campo adversário
	}
	cout << endl;

	for(int i = 0; i < 10; ++i) {
		cout << " " << i << " "; // Índice de linha do seu campo
		for(int j = 0; j < 10; ++j) {
			cout << campo[i][j].get<string>() << " "; // Seu campo
		}

		cout << "    "; // Espaço entre os campos

		cout << " " << i << " "; // Índice de linha do campo adversário
		for(int j = 0; j < 10; ++j) {
			cout << campoAdv[i][j].get<string>() << " "; // Campo adversário
		}
		cout << endl;
	}
	cout << "\n";
}

void criarArquivoJSON(vector<json> &navios, string nome) {
	ofstream arquivo(nome);

	arquivo << "[\n";
	for(int i = 0; i < navios.size(); ++i) {
		arquivo << "    {\n";
		arquivo << "        \"tipo\": \"" << navios[i]["tipo"].get<string>() << "\",\n";
		arquivo << "        \"posicoes\": ";
		arquivo << navios[i]["posicoes"];
		arquivo << "\n    }";
		if(i != navios.size() - 1) {
			arquivo << ",";
		}
		arquivo << "\n";
	}
	arquivo << "]";
}

void posicoesNavios(vector<vector<int>> &todasPosicoes, vector<vector<int>> &todasPosicoesAdversario) {
	json navios, naviosAdversarios;

	ifstream naviosAdv("naviosAdv.json");
	naviosAdv >> naviosAdversarios;
	naviosAdv.close();

	ifstream meusNavios("navios.json");
	meusNavios >> navios;
	meusNavios.close();

	for(const auto &navio : naviosAdversarios) {
		for(const auto &pos : navio["posicoes"]) {
			todasPosicoesAdversario.push_back(pos);
		}
	}

	for(const auto &navio : navios) {
		for(const auto &pos : navio["posicoes"]) {
			todasPosicoes.push_back(pos);
		}
	}
}

int verificarTiro(string tiro, bool meuTiro, vector<vector<int>> &posicoesJogadas, vector<vector<int>> &todasPosicoes) {
	json navios;

	if(meuTiro) {
		ifstream arquivoNav("naviosAdv.json");
		arquivoNav >> navios;
		arquivoNav.close();

	} else {
		ifstream arquivoNav("navios.json");
		arquivoNav >> navios;
		arquivoNav.close();
	}

	vector<int> posTiro = {tiro[0] - '0', tiro[1] - '0'};
	json navioAtingido;

	bool acertou = false;

	for(const auto &pos : posicoesJogadas) {
		if(pos == posTiro) {
			return 1;
		}
	}

	for(const auto &navio : navios) {
		for(const auto &pos : navio["posicoes"]) {
			if(pos == posTiro) {
				acertou = true;
				navioAtingido = navio;
				break;
			}
		}
	}

	posicoesJogadas.push_back(posTiro);

	mostrarNavios();
	exibirCamposPosTiro(todasPosicoes, navios, posTiro, meuTiro);

	cout << "\n";

	int celulas = 0;

	if(acertou) {
		if(navioAtingido["tipo"].get<string>() == "porta-avioes")
			celulas = 5;
		else if(navioAtingido["tipo"].get<string>() == "encouracado")
			celulas = 4;
		else if(navioAtingido["tipo"].get<string>() == "cruzador")
			celulas = 3;
		else if(navioAtingido["tipo"].get<string>() == "destroier")
			celulas = 2;
	}

	if(todasPosicoes.size() == 0) {
		if(meuTiro) {
			if(acertou) {
				cout << "> Você atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ACERTOU! Navio: " << navioAtingido["tipo"].get<string>() << " [" << celulas << " células]" << "\n";
			} else {
				cout << "> Você atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ERROU!" << "\n";
			}
		} else {
			if(acertou) {
				cout << "> Adversário atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ACERTOU! Navio: " << navioAtingido["tipo"].get<string>() << " [" << celulas << " células]" << "\n";
			} else {
				cout << "> Adversário atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ERROU!" << "\n";
			}
		}
		return 0;
	}

	if(meuTiro) {
		if(acertou) {
			cout << "> Você atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ACERTOU! Navio: " << navioAtingido["tipo"].get<string>() << " [" << celulas << " células]" << "\n";
			cout << "> Aguardando adversário atirar..." << "\n";
		} else {
			cout << "> Você atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ERROU!" << "\n";
			cout << "> Aguardando adversário atirar..." << "\n";
		}
	} else {
		if(acertou) {
			cout << "> Adversário atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ACERTOU! Navio: " << navioAtingido["tipo"].get<string>() << " [" << celulas << " células]" << "\n";
			cout << "> É a sua vez de atirar!" << "\n";
			cout << "> Digite [0-9][0-9] (ordem linha/coluna) para tentar acertar um navio" << "\n";
		} else {
			cout << "> Adversário atirou na posição " << posTiro[0] << ":" << posTiro[1] << " e ERROU!" << "\n";
			cout << "> É a sua vez de atirar!" << "\n";
			cout << "> Digite [0-9][0-9] (ordem linha/coluna) para tentar acertar um navio" << "\n";
		}
	}

	return 0;
}

void enviarNavios(int &rem_sockfd, vector<json> &navios) {
	string naviosJSON = json(navios).dump();
	cout << "> Enviando os Navios..." << "\n";

	// Enviar o conteúdo do arquivo JSON
	send(rem_sockfd, naviosJSON.c_str(), naviosJSON.size(), 0);

	// Enviar o EOF para indicar que o envio terminou
	string eof = "EOF";
	send(rem_sockfd, eof.c_str(), eof.size(), 0); // Envia EOF para indicar o final da transmissão
}

void receberNavios(int &rem_sockfd) {
	vector<char> buffer(ECHOMAX);
	string recebido;

	// Recebe os dados até encontrar o EOF
	while(true) {
		int bytes = recv(rem_sockfd, buffer.data(), buffer.size(), 0);

		recebido.append(buffer.data(), bytes);

		// Verifica se o EOF foi recebido
		if(recebido.find("EOF") != string::npos) {
			// Remover o "EOF" da string
			recebido = recebido.substr(0, recebido.find("EOF"));
			break; // Sai do loop, já leu todos os dados
		}
	}

	cout << "> Recebendo os Navios do Adversário..." << "\n";
	// Aqui, 'recebido' contém o JSON completo sem o "EOF"
	vector<json> navios = json::parse(recebido);

	// Criar o arquivo JSON na máquina do jogador
	criarArquivoJSON(navios, "naviosAdv.json");

	vector<vector<int>> todasPosicoes;
}

void atualizarPosicoesJogadas(vector<vector<int>> &posicoesJogadas, vector<vector<int>> &posicoesJogadasAdv) {
	json campo, campoAdv;

	ifstream campoAdversario("campoAdv.json");
	ifstream meuCampo("campo.json");

	if(campoAdversario.is_open()) {
		campoAdversario >> campoAdv;
	}

	if(meuCampo.is_open()) {
		meuCampo >> campo;
	}

	campoAdversario.close();
	meuCampo.close();

	for(int i = 0; i < campo.size(); ++i) {
		for(int j = 0; j < campo[i].size(); ++j) {
			if(campo[i][j] != "🟦") {
				vector<int> pos = {i, j};
				posicoesJogadasAdv.push_back(pos);
			}
		}
	}

	for(int i = 0; i < campoAdv.size(); ++i) {
		for(int j = 0; j < campoAdv[i].size(); ++j) {
			if(campoAdv[i][j] != "🟦") {
				vector<int> pos = {i, j};
				posicoesJogadas.push_back(pos);
			}
		}
	}
}

bool verificarFinalizacaoPartida(vector<vector<int>> &posicoesJogadas, vector<vector<int>> &posicoesJogadasAdv, vector<vector<int>> &todasPosicoes, vector<vector<int>> &todasPosicoesAdversario) {
	vector<vector<int>> temp, tempAdv;

	for(const auto &pos : todasPosicoes) {
		for(const auto &posJogada : posicoesJogadasAdv) {
			if(posJogada == pos) {
				temp.push_back(pos);
			}
		}
	}

	for(const auto &pos : todasPosicoesAdversario) {
		for(const auto &posJogada : posicoesJogadas) {
			if(posJogada == pos) {
				tempAdv.push_back(pos);
			}
		}
	}

	if(temp.size() == todasPosicoes.size() || tempAdv.size() == todasPosicoesAdversario.size()) {
		return true;
	}

	return false;
}

void serverTCP(int argc, char *argv[], vector<json> &navios, bool &continuarPartidaAnterior) {

	int loc_sockfd, loc_newsockfd;
	socklen_t tamanho;
	struct sockaddr_in loc_addr;

	if(argc != 2) {
		printf("Parametros: <local_port> \n");
		exit(1);
	}

	loc_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(loc_sockfd < 0) {
		perror("Criando stream socket");
		exit(1);
	}

	/* Construcao da estrutura do endereco local */
	/* Preenchendo a estrutura socket loc_addr (familia, IP, porta) */
	loc_addr.sin_family = AF_INET;				/* familia do protocolo*/
	loc_addr.sin_addr.s_addr = INADDR_ANY;		/* endereco IP local */
	loc_addr.sin_port = htons(atoi(argv[1])); /* porta local  */
	bzero(&(loc_addr.sin_zero), 8);

	if(bind(loc_sockfd, (struct sockaddr *)&loc_addr, sizeof(struct sockaddr)) < 0) {
		perror("Ligando stream socket");
		exit(1);
	}

	listen(loc_sockfd, 1);

	printf("> aguardando conexao\n");

	tamanho = sizeof(struct sockaddr_in);

	loc_newsockfd = accept(loc_sockfd, (struct sockaddr *)&loc_addr, &tamanho);

	vector<char> linha;
	string mensagem;

	enviarNavios(loc_newsockfd, navios);
	receberNavios(loc_newsockfd);

	vector<json> naviosAdv;
	
	lerArquivoNavios(naviosAdv, "naviosAdv.json");

	vector<vector<int>> todasPosicoes;
	vector<vector<int>> todasPosicoesAdversario;

	posicoesNavios(todasPosicoes, todasPosicoesAdversario);

	vector<vector<int>> posicoesJogadas;
	vector<vector<int>> posicoesJogadasAdv;

	bool finalizacaoPartida = false;

	if(continuarPartidaAnterior) {
		atualizarPosicoesJogadas(posicoesJogadas, posicoesJogadasAdv);
		finalizacaoPartida = verificarFinalizacaoPartida(posicoesJogadas, posicoesJogadasAdv, todasPosicoes, todasPosicoesAdversario);
	}

	bool verificacao;
	bool sair;

	if(!finalizacaoPartida) {
		mostrarNavios();
		exibirCamposInicial();
		cout << "> Você começa jogando!" << "\n";
		cout << "> Digite [0-9][0-9] (ordem linha/coluna) para tentar acertar um navio!" << "\n";
	} else {
		mostrarNaviosFimPartida(navios, naviosAdv);
		exibirCamposInicial();
		cout << "Partida já finalizada anteriormente!" << "\n";
	}

	while(finalizacaoPartida == false) {

		linha.clear();
		linha.resize(ECHOMAX);

		do {
			cout << "> ";
			verificacao = false;
			sair = false;

			getline(cin, mensagem);

			if(verificarMensagem(mensagem)) {
				int retorno = verificarTiro(mensagem, true, posicoesJogadas, todasPosicoesAdversario);
				if(retorno == 1) {
					cout << "> Tiro nessa posição já realizado! Informe outra posição" << "\n";
					continue;
				} else if(todasPosicoesAdversario.size() == 0) {
					mostrarNaviosFimPartida(navios, naviosAdv);
					cout << "> Você ganhou!!! 😃" << "\n";
					sair = true;
				}
				linha.resize(mensagem.size());
				copy(mensagem.begin(), mensagem.end(), linha.begin());
				send(loc_newsockfd, linha.data(), linha.size(), 0);
				verificacao = true;
			} else {
				cout << "> Formato da mensagem deve ser -> [0-9][0-9] (ordem linha/coluna)" << "\n";
			}
		} while(verificacao == false);

		if(sair)
			break;

		linha.clear();
		linha.resize(ECHOMAX);

		recv(loc_newsockfd, linha.data(), linha.size(), 0);

		verificarTiro(string(linha.data()), false, posicoesJogadasAdv, todasPosicoes);

		if(todasPosicoes.size() == 0) {
			mostrarNaviosFimPartida(navios, naviosAdv);
			cout << "> Você perdeu!!! 😢" << "\n";
			break;
		}
	};

	close(loc_sockfd);
	close(loc_newsockfd);
}

void clientTCP(int argc, char *argv[], vector<json> &navios, bool &continuarPartidaAnterior) {
	char *rem_hostname;
	int rem_port;

	struct sockaddr_in rem_addr;
	int rem_sockfd;

	if(argc != 3) {
		cout << "Parametros:<remote_host> <remote_port>" << "\n";
		exit(1);
	}

	rem_hostname = argv[1];
	rem_port = atoi(argv[2]);
	rem_addr.sin_family = AF_INET;							 /* familia do protocolo*/
	rem_addr.sin_addr.s_addr = inet_addr(rem_hostname); /* endereco IP local */
	rem_addr.sin_port = htons(rem_port);					 /* porta local  */

	rem_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(rem_sockfd < 0) {
		perror("Criando stream socket");
		exit(1);
	}

	cout << "> Conectando no servidor " << rem_hostname << ":" << rem_port << "\n";

	if(connect(rem_sockfd, (struct sockaddr *)&rem_addr, sizeof(rem_addr)) < 0) {
		perror("Conectando stream socket");
		exit(1);
	}

	vector<vector<int>> posicoesJogadas;
	vector<vector<int>> posicoesJogadasAdv;

	vector<char> linha;
	string mensagem;

	receberNavios(rem_sockfd);
	enviarNavios(rem_sockfd, navios);

	vector<json> naviosAdv;

	lerArquivoNavios(naviosAdv, "naviosAdv.json");

	vector<vector<int>> todasPosicoes;
	vector<vector<int>> todasPosicoesAdversario;

	posicoesNavios(todasPosicoes, todasPosicoesAdversario);

	bool finalizacaoPartida = false;

	if(continuarPartidaAnterior) {
		atualizarPosicoesJogadas(posicoesJogadas, posicoesJogadasAdv);
		finalizacaoPartida = verificarFinalizacaoPartida(posicoesJogadas, posicoesJogadasAdv, todasPosicoes, todasPosicoesAdversario);
	}

	bool verificacao;
	bool sair;

	if(!finalizacaoPartida) {
		mostrarNavios();
		exibirCamposInicial();
		cout << "> Adversário começa jogando..." << "\n";
	}
	else {
		mostrarNaviosFimPartida(navios, naviosAdv);
		exibirCamposInicial();
		cout << "Partida já finalizada anteriormente!" << "\n";
	}

	while(finalizacaoPartida == false) {
		linha.clear();
		linha.resize(ECHOMAX);

		recv(rem_sockfd, linha.data(), linha.size(), 0);

		verificarTiro(string(linha.data()), false, posicoesJogadasAdv, todasPosicoes);

		if(todasPosicoes.size() == 0) {
			mostrarNaviosFimPartida(navios, naviosAdv);
			cout << "> Você perdeu!!! 😢" << "\n";
			break;
		}

		do {
			cout << "> ";
			sair = false;
			verificacao = false;

			getline(cin, mensagem);

			if(verificarMensagem(mensagem)) {
				int retorno = verificarTiro(mensagem, true, posicoesJogadas, todasPosicoesAdversario);
				if(retorno == 1) {
					cout << "> Tiro nessa posição já realizado! Informe outra posição" << "\n";
					continue;
				} else if(todasPosicoesAdversario.size() == 0) {
					mostrarNaviosFimPartida(navios, naviosAdv);
					cout << "> Você ganhou!!! 😃" << "\n";
					sair = true;
				}
				linha.resize(mensagem.size());
				copy(mensagem.begin(), mensagem.end(), linha.begin());
				send(rem_sockfd, linha.data(), linha.size(), 0);
				verificacao = true;
			} else {
				cout << "> Formato da mensagem deve ser -> [0-9][0-9] (ordem linha/coluna)" << "\n";
			}
		} while(verificacao == false);

		if(sair)
			break;
	};

	close(rem_sockfd);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	string conexao, partidaAnterior;
	bool continuarPartidaAnterior;

	vector<vector<int>> todasPosicoes;
	vector<string> tipos = {"porta-avioes", "encouracado", "cruzador", "cruzador", "destroier", "destroier"};
	vector<int> tamanhos = {5, 4, 3, 3, 2, 2};
	vector<json> navios;

	cout << "A partida anterior será continuada? (sim/nao)" << "\n";

	do {
		getline(cin, partidaAnterior);
		if(partidaAnterior == "sim")
			continuarPartidaAnterior = true;
		else if(partidaAnterior == "nao")
			continuarPartidaAnterior = false;
		else
			cout << "Informe sim ou nao" << "\n";

	} while(partidaAnterior != "sim" && partidaAnterior != "nao");

	if(continuarPartidaAnterior) {
		lerArquivoNavios(navios, "navios.json");

		cout << "Será client ou server?" << "\n";
		getline(cin, conexao);

		if(conexao == "server") {
			serverTCP(argc, argv, navios, continuarPartidaAnterior);
		} else if(conexao == "client") {
			clientTCP(argc, argv, navios, continuarPartidaAnterior);
		} else {
			cout << "Deve ser informado client ou server" << "\n";
			exit(1);
		}
	} else {
		for(int i = 0; i < tipos.size(); ++i) {
			gerarPosicoes(tamanhos[i], todasPosicoes, tipos[i], navios);
		}

		criarArquivoJSON(navios, "navios.json");

		criarCampo("campo.json");
		criarCampo("campoAdv.json");

		cout << "Será client ou server?" << "\n";
		getline(cin, conexao);

		if(conexao == "server") {
			serverTCP(argc, argv, navios, continuarPartidaAnterior);
		} else if(conexao == "client") {
			clientTCP(argc, argv, navios, continuarPartidaAnterior);
		} else {
			cout << "Deve ser informado client ou server" << "\n";
			exit(1);
		}
	}

	return 0;
}
