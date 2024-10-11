package com.example.ucurrentapp;


import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class MainActivity extends AppCompatActivity {

    private TextView currentValue;
    private Button button_uA, button_mA, button_auto;
    private String currentUnit = "µA";  // Unidad por defecto
    private String mode = "uA";         // Modo inicial
    private final String esp32_ip = "http://192.168.4.1";  // IP del ESP32

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Inicializar las vistas
        currentValue = findViewById(R.id.current_value);
        button_uA = findViewById(R.id.button_uA);
        button_mA = findViewById(R.id.button_mA);
        button_auto = findViewById(R.id.button_auto);

        // Inicialmente seleccionamos el botón de µA
        selectButton(button_uA);

        // Botón para cambiar al modo µA
        button_uA.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMode("uA");  // Cambiar el modo a µA
                selectButton(button_uA);
            }
        });

        // Botón para cambiar al modo mA
        button_mA.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMode("mA");  // Cambiar el modo a mA
                selectButton(button_mA);
            }
        });

        // Botón para cambiar al modo Auto
        button_auto.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setMode("Auto");  // Cambiar el modo a Auto
                selectButton(button_auto);
            }
        });

        // Actualizar los valores periódicamente
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        updateValuesFromESP();  // Obtener los valores del ESP32
                        Thread.sleep(1000);     // Actualizar cada segundo
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }

    // Cambiar el modo en el ESP32
    private void setMode(String newMode) {
        mode = newMode;  // Actualizar el modo localmente
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    // Construir la URL correcta para cambiar el modo en la ESP32
                    URL url = new URL(esp32_ip + "/setMode?mode=" + mode);
                    HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                    urlConnection.setRequestMethod("GET");
                    urlConnection.connect();

                    // Obtener el código de respuesta
                    int responseCode = urlConnection.getResponseCode();
                    urlConnection.disconnect();

                    // Verificar si la solicitud fue exitosa
                    if (responseCode == 200) {
                        // Actualizar la interfaz en el hilo principal si la solicitud fue exitosa
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                currentValue.setText("Modo cambiado a " + mode);
                            }
                        });
                    } else {
                        // Mostrar un mensaje si la solicitud no fue exitosa
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                currentValue.setText("Error al cambiar el modo, código: " + responseCode);
                            }
                        });
                    }

                } catch (Exception e) {
                    e.printStackTrace();
                    // Mostrar un mensaje si ocurre una excepción
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            currentValue.setText("Error en la solicitud al ESP32: " + e.getMessage());
                        }
                    });
                }
            }
        }).start();
    }

    // Obtener los valores actuales del ESP32
    private void updateValuesFromESP() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    URL url = new URL(esp32_ip + "/data");
                    HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                    urlConnection.setRequestMethod("GET");
                    urlConnection.connect();

                    BufferedReader in = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
                    String jsonData = in.readLine();
                    in.close();
                    urlConnection.disconnect();

                    // Parsear los datos recibidos (corriente, modo y rango)
                    String[] data = jsonData.replace("{", "").replace("}", "").split(",");
                    String currentReading = data[0].split(":")[1];
                    String modeReceived = data[1].split(":")[1].replace("\"", "").trim();
                    String rangeReceived = data[2].split(":")[1].replace("\"", "").trim();

                    // Actualizar la unidad según lo recibido del ESP32
                    currentUnit = rangeReceived;

                    // Actualizar la pantalla con el valor de corriente y la unidad
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            currentValue.setText(String.format("Corriente: %s %s", currentReading, currentUnit));
                        }
                    });

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    // Método para seleccionar un botón visualmente
    private void selectButton(Button selectedButton) {
        // Resetear el estilo de todos los botones
        button_uA.setBackgroundColor(getResources().getColor(android.R.color.darker_gray));
        button_mA.setBackgroundColor(getResources().getColor(android.R.color.darker_gray));
        button_auto.setBackgroundColor(getResources().getColor(android.R.color.darker_gray));

        // Aplicar un estilo visual al botón seleccionado
        selectedButton.setBackgroundColor(getResources().getColor(android.R.color.holo_blue_light));
    }

    // Método para actualizar la pantalla con el valor de corriente y la unidad
    private void updateCurrentDisplay() {
        currentValue.setText(String.format("Corriente: 0.00 %s", currentUnit));  // Mostrar la unidad actual
    }
}
