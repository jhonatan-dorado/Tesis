#include <EtherCard.h>

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };    // Mac de la tarjeta ether... Debe ser unico en la red local

const static uint8_t ip[] = {172,18,56,56};                 // Ip de la tarjeta
const static uint8_t gw[] = {172,18,112,1};                  // Ip de la puerta de enlace
const static uint8_t mask[] = {255,255,248,0};              // Mascara de la tarjea
const static uint8_t dns[] = {172,18,2,5};                // Ip del servidor DNS, si existe
const static uint8_t hisip[] = {8,8,8,8};                   // Ip de la aplicacion remota
const static int hisport = 8080;                            // Puerto de la aplicacion remota

int desencadenado = 4;
int eco = 3;


byte Ethernet::buffer[700];                                 // Buffer de la tarjeta
static uint32_t timer;                                      // Acumulador de tiempo

//const char website[] PROGMEM = "10.14.0.104";               // Nombre del servidor remoto, si es una IP fina puede fallar
const char website[] PROGMEM = "webservice17.herokuapp.com";  // Forma alternativa con nmbre de dominio

static void my_callback (byte status, word off, word len)   // Funcion que se ejecuta al recibir la respuesta de la aplicacion remota
{
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () 
{
  Serial.begin(9600);
  Serial.println("\n[webClient]");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)         // Activa la tarjeta 
    Serial.println( "Error al acceder al controlador ethernet");
  
  if (!ether.staticSetup(ip,gw,dns,mask))                       // Configura manualmente la tarjeta, se puede usar ether.dhcpSetup() si se esta conectado a una red con dhcp
    Serial.println("Error al configurar las IP");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))                                // Trata de cinfigurar la ip del servidor por DNS
  {
    Serial.println("DNS failed");
    ether.copyIp(ether.hisip, hisip);                           // Si falla el DNS se configura manualmente
    ether.hisport = hisport;
  }
   
  ether.printIp("SRV: ", ether.hisip);
  ether.printIp("PORT: ", ether.hisport);
}

void loop () 
{
  ether.packetLoop(ether.packetReceive());                      // Activa el loop principal de la tarjeta
  
  if (millis() > timer)                                         // Verifica que el proceso se ejecte cada 5 segundos
  {
    timer = millis() + 5000;                                    // Lmite a 5 segundos
    Serial.println();
    Serial.print("<<< REQ ");
    
    digitalWrite(desencadenado, LOW);
    delayMicroseconds(2);
    
    digitalWrite(desencadenado, HIGH);
    delayMicroseconds(10);

    
    float tiempo = pulseIn(eco, HIGH);
    float distancia = (tiempo/2.0)/29.0; // conversion a metros
    
    float mi = analogRead(A0);
    float temperatura = (5.0 * mi * 100.0)/1024.0; // conversion a grados centigrados
    
    float turbidez = 0; //Conversion a lumens pendiente
     
    String te = String(temperatura); // conversion a cadene de caracteres
    String pr = String(distancia);
    String tu = String(turbidez);
    
    String peticion = "profundidad="+pr+"&turbides="+tu+"&temperatura="+te+"&caudal=54";
    ether.browseUrl(PSTR("/agregar.php?"), peticion, website, my_callback);       // Envia la peticion
  }
}
