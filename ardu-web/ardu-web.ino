#include "EmonLib.h"
#include <SPI.h>
#include <Ethernet.h>


// Crear una instancia EnergyMonitor
EnergyMonitor energyMonitor;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Direccion Fisica MAC
IPAddress ip(192, 168, 0, 177);                      // IP Local que usted debe configurar

EthernetServer server(80);                             //Se usa el puerto 80 del servidor

int cont_ande = 0;
int cont_generador = 0;

void setup() {
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);

  pinMode(4, INPUT);
  pinMode(3, INPUT);


  // Iniciamos la clase indicando
  // Número de pin: donde tenemos conectado el SCT-013
  // Valor de calibración: valor obtenido de la calibración teórica
  energyMonitor.current(0, 2.63);

  Ethernet.begin(mac, ip); // Inicializa la conexion Ethernet y el servidor
  server.begin();

  Serial.begin(9600);

}

void loop()
{
  double Irms = energyMonitor.calcIrms(1500);
  boolean estado_ande = digitalRead(4);
  boolean estado_generador = digitalRead(3);
  String c_Ande = "";
  if (estado_ande) {
    cont_generador = 0;
    digitalWrite(7, LOW);
    if (cont_ande == 0) {

      //Desactivamos el contactor de generador
      digitalWrite(7, LOW);
      delay(3000);

      //Activamos el contactor de la Ande
      digitalWrite(6, HIGH);

      //Activamos el stop del generador
      digitalWrite(8, HIGH);
      cont_ande = cont_ande + 1;
    }

  } else {
    cont_ande = 0;
    if (cont_generador == 0) {
      //Desactivamos el contactor de la Ande
      digitalWrite(6, LOW);

      delay(3000);

      //Activamos el contactor de generador
      digitalWrite(7, HIGH);

      //Desactivamos el stop del generador
      digitalWrite(8, LOW);
      cont_generador = cont_generador + 1;
    }

  }
  EthernetClient cliente = server.available(); 
  if (cliente) {
    boolean currentLineIsBlank = true;
    String cadena = "";
    while (cliente.connected()) {
      if (cliente.available()) {
        char c = cliente.read();
        if (cadena.length() < 50) {
          cadena.concat(c);

          int posicion = cadena.indexOf("Data=");

          if (cadena.substring(posicion) == "Data=1") {
            start();
          }
          else if (cadena.substring(posicion) == "Data=2") {
            reset();
          }
        }
        if (c == 'n' && currentLineIsBlank) {
          // Enviamos al cliente una respuesta HTTP
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");
          cliente.println();

          //Página web en formato HTML
          cliente.println("<!DOCTYPE html>");
          cliente.println("<html lang='es'>");
          cliente.println("<head>");
          cliente.println("<meta charset='UTF-8'>");
          cliente.println("<meta name=viewport content=width=device-width, initial-scale=1.0>");
          cliente.println("<title>Web Ardu</title>");

          //Codigo CSS
          cliente.println("<style type=\'text/css\'>");
        
            estilos(cliente);

          cliente.println("</style>");


          cliente.println("</head>");

          cliente.println("<body>");

          cliente.println("<div class='container'>");
          cliente.println("<h2>TTA REMOTE</h2>");

          cliente.println("<div class='senhal'>");
          cliente.println("<div id='ande'></div>");
          cliente.println("<div id='generador'></div>");
          cliente.println("</div>");

          cliente.println("<div class='labels'>");
          cliente.println("<label id='l-ande'>Ande</label>");
          cliente.println("<label id='l-generador'>Generador</label>");
          cliente.println("</div>");

          cliente.println("<div class='display'>");
          cliente.println("<b>");
          cliente.println(Irms);
          cliente.println("</b>");
          cliente.println("</div>");

          cliente.println("<div class='buttons'>");

          cliente.print("<button onClick=location.href='./?Data=1'>Start</button>");
          cliente.print("<button onClick=location.href='./?Data=2'>Reset</button>");

          cliente.println("</div>");

          cliente.println("</div>");

          cliente.println("</body>");
          cliente.println("</html>");
          break;
        }
        if (c == 'n') {
          currentLineIsBlank = true;
        }
        else if (c != 'r') {
          currentLineIsBlank = false;
        }
      }
    }
    //Dar tiempo al navegador para recibir los datos
    delay(1);
    cliente.stop();// Cierra la conexión
  }
}

void estilos(EthernetClient cliente) {
  cliente.println("body{");
  cliente.println("background: gray;");
  cliente.println("}");

  cliente.println(".container{");
  cliente.println("width: 100%;");
  cliente.println("text-align: center;");
  cliente.println("margin: auto;");
  cliente.println("}");

  cliente.println(".container .senhal {");
  cliente.println("width: 100%;");
  cliente.println("height: 25vh;");
  cliente.println("display: inline-flex;");
  cliente.println("}");

  cliente.println("#ande, #generador {");
  cliente.println("width: 80px;");
  cliente.println("height: 80px;");
  cliente.println("border-radius: 50%;");
  cliente.println("background-color: green;");
  cliente.println("}");

  cliente.println("#ande{");
  cliente.println("margin-left: 15%;");
  cliente.println("}");

  cliente.println("#generador{");
  cliente.println("margin: 0 20%;");
  cliente.println("}");

  cliente.println(".display{");
  cliente.println("margin: auto;");
  cliente.println("width: 200px;");
  cliente.println("height: 80px;");
  cliente.println("border: solid 2px #000;");
  cliente.println("text-align: center;");
  cliente.println("}");

  cliente.println(".display b{");
  cliente.println("display: block;");
  cliente.println("margin-top: 12%;");
  cliente.println("font-size: 26px;");
  cliente.println("}");

  cliente.println(".labels{");
  cliente.println("position: absolute;");
  cliente.println("top: 25%;");
  cliente.println("left: 15%;");
  cliente.println("width: 300px;");
  cliente.println("font-weight: bold;");
  cliente.println("}");

  cliente.println("#l-ande{");
  cliente.println("margin-left: -20%;");
  cliente.println("}");

  cliente.println("#l-generador{");
  cliente.println("margin-left: 30%;");
  cliente.println("}");

  cliente.println(".buttons{");
  cliente.println("width: 80%;");
  cliente.println("height: 20vh;");
  cliente.println("margin: 20% auto;");
  cliente.println("}");



  cliente.println("button{");
  cliente.println("padding: 20px 25px;");
  cliente.println("}");
}

void reset() {
  digitalWrite(9, HIGH);
  delay(500);
  digitalWrite(9, LOW);
}

void start() {
  digitalWrite(5, HIGH);
  delay(1500);
  digitalWrite(5, LOW);
}
