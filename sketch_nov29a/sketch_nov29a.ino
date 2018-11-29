#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
//defines:
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "MQTTCARMackenzieEnvia"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "MQTTCARMackenzieRecebe"    //tópico MQTT de envio de informações para Broker
#define ID_MQTT  "Rf31889808Mackenzie"     //id mqtt (para identificação de sessão)
                                           //IMPORTANTE: este deve ser único no broker (ou seja, 
                                           //            se um client MQTT tentar entrar com o mesmo 
                                           //            id de outro já conectado ao broker, o broker 
                                           //            irá fechar a conexão de um deles).
 
//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
 
//defines - motores
#define MOTOR_DIRETO     D0
#define MOTOR_ESQUERDO   D1
 
// WIFI
const char* SSID = "Redmi";     //Coloque aqui o SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = ""; //Coloque aqui a senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "iot.eclipse.org"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoMotorDireto = '0';  //variável que armazena o estado atual do motor da direita
char EstadoMotorEsquerdo = '0';  //variável que armazena o estado atual do motor da esquerda
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutputs(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    InitOutputs();
    initSerial();
    initWiFi();
    initMQTT();
}
  
//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI- Robo IoT com NodeMCU -----");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
  
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma ação dependendo da string recebida:
    //-----------------------------------------------------
    //     Mensagem recebida       |       Ação tomada
    //-----------------------------------------------------
    //             F               | O robô vai para frente
    //             D               | O robô vai para a direita
    //             E               | O robô vai para a esquera
    //             P               | O robô para imediatamente
     Serial.print("MQTT"+msg);
    if (msg.equals("F"))
    {
        //para ir para frente, os dois motores são ligados
        digitalWrite(MOTOR_DIRETO, HIGH);
        digitalWrite(MOTOR_ESQUERDO, HIGH);
  
        EstadoMotorDireto = '1';
        EstadoMotorEsquerdo = '1';
    }
 
    if (msg.equals("D"))
    {
        //para ir para a direita, somente o motor da esquerda é ligado
        digitalWrite(MOTOR_DIRETO, LOW);
        digitalWrite(MOTOR_ESQUERDO, HIGH);
  
        EstadoMotorDireto = '0';
        EstadoMotorEsquerdo = '1';
    }
 
    if (msg.equals("E"))
    {
        //para ir para a esquerda, somente o motor da direita é ligado
        digitalWrite(MOTOR_DIRETO, HIGH);
        digitalWrite(MOTOR_ESQUERDO, LOW);
  
        EstadoMotorDireto = '1';
        EstadoMotorEsquerdo = '0';
    }
 
    if (msg.equals("P"))
    {
        //para parar, os dois motores são desligados
        digitalWrite(MOTOR_DIRETO, LOW);
        digitalWrite(MOTOR_ESQUERDO, LOW);
  
        EstadoMotorDireto = '0';
        EstadoMotorEsquerdo = '0';
    }
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
    char EstadosMotores[3];
 
    EstadosMotores[0] = EstadoMotorDireto;
    EstadosMotores[1] = '-';
    EstadosMotores[2] = EstadoMotorEsquerdo;
 
    MQTT.publish(TOPICO_PUBLISH, EstadosMotores);
    Serial.println("- Estados dos motores enviados ao broker!");
    delay(1000);
}
 
//Função: inicializa os outputs em nível lógico baixo (desliga os dois motores)
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutputs(void)
{
    pinMode(MOTOR_DIRETO, OUTPUT);
    pinMode(MOTOR_ESQUERDO, OUTPUT);
     
    digitalWrite(MOTOR_DIRETO, LOW);          
    digitalWrite(MOTOR_ESQUERDO, LOW);          
}
 
//programa principal
void loop() 
{   
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
 
    //envia o status de todos os outputs para o Broker no protocolo esperado
    EnviaEstadoOutputMQTT();
 
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}
