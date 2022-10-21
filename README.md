# bot para comprobar si un dominio .cl esta disponible en nic.cl

Comprueba cada cierto periodo de tiempo deinido por el usuario si el dominio esta disponible usando el buscador de NIC Chile y avisa a travez
de un bot de Telegram cuando lo este.


## Uso:

```
./cl_bot [telegram_bot_key] [chat_id/user_id] [dominio]
```

(dominio sin el .cl ni www.)


## Preparacion

### Obtener el Token de la API de bots de Telegram:

Un token es una cadena de caracteres que auntentican al bot (no la cuenta) en la API de bots.  
Cada bot tiene un token unico que se puede revocar en cualquier momento.

Para obtenerlo tienes que contactar en la aplicacion de Telegram a @BotFater.  
Ingresa el comando /newbot y sigue los pasos hasta obtener un nuevo token.

Guia paso a paso, en Ingles: [Aqui](https://core.telegram.org/bots/features#creating-a-new-bot).

Ejemplo de como se ve un token:

```
4839574812:AAFD39kkdpWt3ywyRZergyOLMaJhac60qc
```

### ID de chat de Telegram

Puede ser tu ID de usuario de Telegram o el nombre del grupo donde quieres que te notifique.


### Para que el bot pueda avisarte tienes que haber hablado por lo menos una vez con el.

    NOTA: Si es un grupo, tienes que agregar al bot como admin.



## Instalar 


### Linux (Ubuntu) y Raspberry PI

1. Clonar el repositorio:

```
git clone https://github.com/jorgesc231/cl_bot.git
```


2. Instalar dependencias:

```
sudo apt install build-essential gcc libcurl4-openssl-dev libcurl4
```


3. Compilar:

```
gcc -O2 -Wall cl_bot.c -o cl_bot -lcurl
```

(NOTA: Solo lo probe en la Raspberry Pi 1 B)

## Ejecutar 

Para que el bot funcione el dispositio tiene que estar encendido.

Es recomendable usarlo en una Raspberry PI o un servidor.


### Usando Cron

Crear un crontab para elegir cuando ejecutarlo:

```
crontab -e
```

Para que compruebe cada 15 minutos, agregar esto al final del archivo y guardar:

    */15 * * * * [ruta_absoluta_al_binario] [API_key] [chat_id] [dominio]  >/dev/null 2>&1

Reemplaza los campos por lo que corresponda.


Para ejecutarlo cada minuto:

    * * * * * [ruta_absoluta_al_binario] [API_key] [chat_id] [dominio]  >/dev/null 2>&1

Cada 30 minutos:

    30 * * * * [ruta_absoluta_al_binario] [API_key] [chat_id] [dominio]  >/dev/null 2>&1
    
Cada hora:

    0 * * * * [ruta_absoluta_al_binario] [API_key] [chat_id] [dominio]  >/dev/null 2>&1

Cada dia a medianoche:

    0 0 * * * [ruta_absoluta_al_binario] [API_key] [chat_id] [dominio]  >/dev/null 2>&1


(Para mas opciones buscar un tutorial de cron en internet.)


#### Si se quiere dejar de monitorizar el dominio, ejecuta:

    crontab -r


### Sin usar Cron

TODO
