
// Comenta esta linea para no tener que usar cron
#define _USE_CRON_
/*
* 
*/

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>


// resizable buffer
typedef struct {
    char *buf;
    size_t size;
} memory;

// Constant URLs
#define TELEGRAM_API "https://api.telegram.org/bot%s/sendMessage"
#define NIC_SEARCH_URL "https://www.nic.cl/registry/Whois.do?d=%s&buscar=Submit+Query&a=inscribir"

#define SLEEP_SECONDS 60

const char *tg_api_key;
const char *chat_id;

char url[4096] = {0};
memory* mem;

CURL *telegram_handle = NULL;
CURL *domain_handle = NULL;


size_t grow_buffer(void* contents, size_t sz, size_t nmemb, void *ctx)
{
    size_t realsize = sz * nmemb;
    memory *mem = (memory*) ctx;
    
    char *ptr = realloc (mem->buf, mem->size + realsize);
    
    if (!ptr) {
        // out of memory
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->buf = ptr;
    memcpy(&(mem->buf[mem->size]), contents, realsize);
    mem->size += realsize;
    
    return realsize;
}


int send_telegram_message(CURL* handle, const char* msg)
{
    if (handle) {
        char api_url[2048];
        char post_data[4096];
        
        snprintf(post_data, 4096, "{\"chat_id\":\"%s\",\"text\":\"%s\",\"parse_mode\":\"Markdown\"}", chat_id, msg);
        
        snprintf(api_url, 2048, TELEGRAM_API, tg_api_key);
        
        struct curl_slist *hs = NULL; 
        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, hs);
        
        curl_easy_setopt(handle, CURLOPT_URL, api_url);
        
        /* send data from the local stack */
        curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, post_data);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, stderr);
        
        if (curl_easy_perform(handle) == CURLE_OK)
        {
            return 1;
        } else {
            fprintf(stderr, "\nError al conectar con la API de Telegram\n");
            return 0;
        }
        
    } else return 0;
}

int check_domain (CURL* handle, char *domain)
{
    char *effective_url = NULL;
    long res_status = 0;
    
    snprintf(url, 4096, NIC_SEARCH_URL, domain);
    
    if (handle) {
        // Importante: use HTTP2 over HTTPS
        curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
        curl_easy_setopt(handle, CURLOPT_URL, url);
        
        // buffer body
        memory *mem = malloc (sizeof(memory));
        mem->size = 0;
        mem->buf = malloc(1);
        
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, grow_buffer);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, mem);
        curl_easy_setopt(handle, CURLOPT_PRIVATE, mem);
        
        // for completeness
        curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 1L);
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 2L);
        curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(handle, CURLOPT_FILETIME, 1L);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:98.0) Gecko/20100101 Firefox/98.0");
        curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        curl_easy_setopt(handle, CURLOPT_UNRESTRICTED_AUTH, 1L);
        curl_easy_setopt(handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        curl_easy_setopt(handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 0L);
        
        
        if (curl_easy_perform(handle) == CURLE_OK)
        {
            
            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res_status);
            curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &effective_url);
            
            if (res_status == 200)
            {
                //printf("HTTP 200: %s\n", effective_url);
                
                return 0;
                
            } else if (res_status == 302) {
                
                //printf("HTTP 302: %s\n", effective_url);
                
                char msg_buffer[4096];
                snprintf(msg_buffer, 4096, "El diminio: %s se puede registrar [Aqui](%s)", domain, effective_url);
                
                send_telegram_message(telegram_handle, msg_buffer);
                
                return 1;
                
            } else {
                printf("HTTP %d: %s\n", (int) res_status, url);
            }
            
            return 0;
        }
        
    } 
    
    return 0;
}



int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "No se ingresaron los parametros correctos:!\n");
        printf("\n%s [telegram_token] [chat_id] [dominio]\n\n", argv[0]);
        
        exit(1);
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
	
    // Init curl handlers
    telegram_handle = curl_easy_init();
    domain_handle = curl_easy_init();
    
    time_t log_time = time(NULL); 
    
    tg_api_key = argv[1];
    chat_id = argv[2];
    
#ifdef _USE_CRON_
    
    if (check_domain(domain_handle, argv[3])) 
    {
        printf("[%.24s] - Dominio %s.cl Disponible!\n", ctime(&log_time), argv[3]);
        
    } else {
        printf("[%.24s] - Dominio %s.cl aun no disponible\n", ctime(&log_time), argv[3]);
    }
    
#else
    
    int running = 1;
    
    while (running)
    {
        log_time = time(NULL);
        
        if (check_domain(domain_handle, argv[3])) 
        {
            printf("[%.24s] - Dominio %s.cl Disponible!\n", ctime(&log_time), argv[3]);
            break;
        }
        
        printf("[%.24s] - Dominio %s.cl aun no disponible\n", ctime(&log_time), argv[3]);
        
        sleep(SLEEP_SECONDS);
    }
#endif
    
    
    curl_easy_cleanup(telegram_handle);
    curl_easy_cleanup(domain_handle);
    
    return 0;
}
