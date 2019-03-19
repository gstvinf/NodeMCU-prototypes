//Programa: NodeMCU e MQTT - Controle e Monitoramento IoT
//Autor: Pedro Bertoleti
 
#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient

#define TOPICO_LAMPADA   "gstvinf/lampada"

#define ID_MQTT  "HomeAut"  // deve ser único

#define buttonPin    5
#define ledPin    4

bool ledState = false;
int buttonState = 0;
int lastButtonState = 1;
String mensagem = "";
 
 
// WIFI
//const char* SSID = "MAXIMA-DESCANSO";
//const char* PASSWORD = "@inicio1234";
const char* SSID = "SSID";
const char* PASSWORD = "SENHA";
  
// MQTT
const char* BROKER_MQTT = "iot.eclipse.org";
int BROKER_PORT = 1883;
 
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

/**
 * Funções
 */
void setup() 
{
    //inicializações:
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
}
  

void initSerial() 
{
    Serial.begin(115200);
}
 
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}
  
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) {
       char c = (char)payload[i];
       msg += c;
    }

    if (msg.equals("1")) {
        digitalWrite(ledPin, HIGH);
        ledState = true;
    }
 
    if (msg.equals("0")) {
        digitalWrite(ledPin, LOW);
        ledState = false;
    }
    mensagem = msg;
//    Serial.print("Leitura da mensagem: ");
//    Serial.println(msg);
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_LAMPADA); 
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED){
        return;
    }
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) {
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
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
//Função: envia ao Broker o estado atual do output 
void EnviaEstadoOutputMQTT(void) {
    if (ledState){
      MQTT.publish(TOPICO_LAMPADA, "1");
    } 
    if (!ledState) {
      MQTT.publish(TOPICO_LAMPADA, "0");
    }
    
    Serial.print("ALERTA DE ENVIO: ledState: ");
    Serial.println(ledState);
}
 
//Função: inicializa o output em nível lógico baixo
void InitOutput(void)
{
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);  

    pinMode(buttonPin, INPUT);
    digitalWrite(buttonPin, LOW);
}
 
 
//programa principal
void loop() 
{   
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
    
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();


    lastButtonState = buttonState;
    buttonState = digitalRead(buttonPin);
    
    if (lastButtonState != buttonState) {
      ledState = !ledState;
      delay(10);
      EnviaEstadoOutputMQTT();
    }

    if(mensagem != String(ledState)) {
      EnviaEstadoOutputMQTT();
    }

    if (ledState) {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }

    
    Serial.print("LOG: LED: ");
    if (ledState) { Serial.print("ligado    "); } 
    else          { Serial.print("desligado "); } 
    Serial.print(" ");
    Serial.print("mensagem: ");
    Serial.print(mensagem);
    Serial.print(" ");
    Serial.print("estadoBotao: ");
    Serial.print(buttonState);
    Serial.println("");
}
