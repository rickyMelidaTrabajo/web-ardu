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
  String c_Ande = "#222;";
  String c_Generador = "#222;";


  if (estado_ande) {
    cont_generador = 0;
    c_Ande = "#00ff00;";

    digitalWrite(7, LOW);

    if (cont_ande == 0) {

      //Desactivamos el contactor de generador
      digitalWrite(7, LOW);
      c_Generador = "#222;";

      delay(3000);

      //Activamos el contactor de la Ande
      digitalWrite(6, HIGH);

      //Activamos el stop del generador
      digitalWrite(8, HIGH);
      cont_ande = cont_ande + 1;
    }

  } else {

    if(estado_generador) {
      c_Generador = "#00ff00;";
    }
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

          //Si no tenemos ande ni generador podemos hacer arrancar el generador
          if (!estado_generador && !estado_ande) {
            if (cadena.substring(posicion) == "Data=1") {
              start();
            }
          }

          if (cadena.substring(posicion) == "Data=2") {
            reset();
          }
        }
        if (c == 'n' && currentLineIsBlank) {
          // Enviamos al cliente una respuesta HTTP
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");
          cliente.println("Connection: close");
          cliente.println();


          //Página web en formato HTML
          cliente.println(F("<!DOCTYPE html>"));
          cliente.println(F("<html lang='es'>"));
          cliente.println(F("<head>"));
          cliente.println(F("<meta charset='UTF-8'>"));
          cliente.println(F("<meta name=viewport content=width=device-width, initial-scale=1.0>"));
          cliente.println(F("<title>Web Ardu</title>"));

          //Codigo CSS
          cliente.println(F("<style type=\'text/css\'>"));

          estilos(cliente, c_Ande, c_Generador);

          cliente.println(F("</style>"));


          cliente.println(F("</head>"));

          cliente.println(F("<body>"));

          cliente.println(F("<div class='container'>"));
          cliente.println(F("<h2>TTA REMOTE</h2>"));

          cliente.println(F("<div class='senhal'>"));
          cliente.println(F("<div id='ande'></div>"));
          cliente.println(F("<div id='generador'></div>"));
          cliente.println(F("</div>"));

          cliente.println(F("<div class='labels'>"));
          cliente.println(F("<label id='l-ande'>Ande</label>"));
          cliente.println(F("<label id='l-generador'>Generador</label>"));
          cliente.println(F("</div>"));

          cliente.println(F("<div class='display'>"));
          cliente.println(F("<b>"));
          cliente.println(Irms);
          cliente.println(F("</b>"));
          cliente.println(F("</div>"));

          cliente.println(F("<a href='./' class='refresh'>Actualizar</a>"));
          
          cliente.println(F("<div class='buttons'>"));

          if (estado_generador || estado_ande) {
            cliente.print(F("<button disabled onClick=location.href='./?Data=1'>Start</button>"));
          } else {
            cliente.print(F("<button onClick=location.href='./?Data=1'>Start</button>"));
          }

          cliente.print(F("<button onClick=location.href='./?Data=2'>Reset</button>"));

          cliente.println(F("</div>"));

          cliente.println(F("</div>"));

          cliente.println(F("</body>"));
          cliente.println(F("</html>"));
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

void estilos(EthernetClient cliente, String c_Ande, String c_Generador) {
  cliente.println(F("body{"));
  cliente.println(F("background: #222;"));
  cliente.println(F("color: #fff;"));
  cliente.println(F("}"));

  cliente.println(F(".container{"));
  cliente.println(F("width: 100%;"));
  cliente.println(F("text-align: center;"));
  cliente.println(F("margin: auto;"));
  cliente.println(F("}"));

  cliente.println(F(".container .senhal {"));
  cliente.println(F("width: 100%;"));
  cliente.println(F("height: 25vh;"));
  cliente.println(F("display: inline-flex;"));
  cliente.println(F("}"));

  cliente.println(F("#ande, #generador {"));
  cliente.println(F("width: 80px;"));
  cliente.println(F("height: 80px;"));
  cliente.println(F("border: solid 2px #fff;"));
  cliente.println(F("border-radius: 50%;"));
  //cliente.println("background-color: green;"));
  cliente.println(F("}"));

  cliente.println(F("#ande{"));
  cliente.print(F("background-color:")); cliente.println(c_Ande);
  cliente.println(F("margin-left: 15%;"));
  cliente.println(F("}"));

  cliente.println(F("#generador{"));
  cliente.print(F("background-color:")); cliente.println(c_Generador);
  cliente.println(F("margin: 0 20%;"));
  cliente.println(F("}"));

  cliente.println(F(".display{"));
  cliente.println(F("margin: 5px auto;"));
  cliente.println(F("width: 200px;"));
  cliente.println(F("height: 80px;"));
  cliente.println(F("border: solid 2px #000;"));
  cliente.println(F("text-align: center;"));
  cliente.println(F("}"));

  cliente.println(F(".display b{"));
  cliente.println(F("display: block;"));
  cliente.println(F("margin-top: 12%;"));
  cliente.println(F("font-size: 26px;"));
  cliente.println(F("}"));

  cliente.println(F(".labels{"));
  cliente.println(F("position: absolute;"));
  cliente.println(F("top: 30%;"));
  cliente.println(F("left: 15%;"));
  cliente.println(F("width: 300px;"));
  cliente.println(F("font-weight: bold;"));
  cliente.println(F("}"));

  cliente.println(F("#l-ande{"));
  cliente.println(F("margin-left: -20%;"));
  cliente.println(F("}"));

  cliente.println(F("#l-generador{"));
  cliente.println(F("margin-left: 30%;"));
  cliente.println(F("}"));

  cliente.println(F(".buttons{"));
  cliente.println(F("width: 80%;"));
  cliente.println(F("height: 20vh;"));
  cliente.println(F("margin: 20% auto;"));
  cliente.println(F("}"));



  cliente.println(F("button{"));
  cliente.println(F("width: 100px;"));
  cliente.println(F("height: 50px;"));
  cliente.println(F("background-color: #0066ff;"));
  cliente.println(F("border: solid 2px #000;"));
  cliente.println(F("border-radius: 20px;"));
  cliente.println(F("margin: 6%;"));
  cliente.println(F("}"));

  cliente.println(F(".refresh {"));
  cliente.println(F("font-size: 14px;"));
  cliente.println(F("border-radius: 5px;"));
  cliente.println(F("margin: 5%;"));
  cliente.println(F("width: 400px;"));
  cliente.println(F("height: 10px;"));
  cliente.println(F("padding: 5px;"));
  cliente.println(F("border: solid 2px #000;"));
  cliente.println(F("text-decoration: none;"));
  cliente.println(F("color: #fff;"));
  cliente.println(F("}"));
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
