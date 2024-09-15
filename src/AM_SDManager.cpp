#include "AM_SDManager.h"

#include "AM_SDK_PicoWiFi.h"

#include "f_util.h"
#include "ff.h"

SDManager::SDManager(AMController *pico)
{
    this->pico = pico;
}

bool SDManager::endsWith(const char *base, const char *str)
{
    int blen = strlen(base);
    int slen = strlen(str);
    if (slen <= blen)
    {
        return (0 == strcmp(base + blen - slen, str));
    }
    return false;
}

void SDManager::process_sd_request(char *variable, char *value)
{
    DEBUG_printf("\t** processSD **\n");
    DEBUG_printf("\t\t** Variable %s - Value %s\n", variable, value);

    if (strcmp(variable, "SD") == 0)
    {
        dir();
    }

    if (strcmp(variable, "$SDDL$") == 0 && strlen(value) > 0)
    {
        transmit_file(value);
    }
}

void SDManager::dir()
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    fr = f_findfirst(&dir, &fno, "/", "*");
    if (FR_OK != fr)
        panic("f_open(%s) error: %s (%d)\n", "/", FRESULT_str(fr), fr);

    while (fr == FR_OK && fno.fname[0])
    { /* Repeat while an item is found */

        if (fno.fname[0] != '.' && endsWith(fno.fname, ".txt"))
        {
            DEBUG_printf("%s\n", fno.fname);
            pico->write_message("SD", fno.fname);
        }
        fr = f_findnext(&dir, &fno); /* Search for next item */
    }
    pico->write_message("SD", "$EFL$");

    f_closedir(&dir);

    f_unmount("");
}

bool SDManager::transmit_file(char *filename)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;
    char buffer[64];

    DEBUG_printf("Sending file %s\n", filename);

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    fr = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);
    if (fr != FR_OK)
    {
        return false;
    }

    pico->write_message("SD", "$C$");

    int chunk = 0;
    while (!f_eof(&fil))
    {
        uint size;
        fr = f_read(&fil, &buffer, sizeof(buffer), &size);
        if (fr != FR_OK)
        {
            f_close(&fil);
            f_unmount("");

            return false;
        }
        DEBUG_printf("Chunk %d\n", chunk++);
        pico->write_message_buffer(buffer, size);
    }
    pico->write_message("SD", "$E$");

    DEBUG_printf("\nFile %s completed\n", filename);

    f_close(&fil);

    f_unmount("");

    return true;
}

// Is this used somewhere?
bool SDManager::append(char *filename, uint8_t *byte, unsigned int size)
{
    FRESULT fr;
    FIL fil;

    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
        return false;
    }

    unsigned int written_bytes;

    fr = f_write(&fil, byte, size, &written_bytes);
    if (fr != FR_OK)
    {
        DEBUG_printf("f_write error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }

    f_close(&fil);

    return (written_bytes == size);
}

void SDManager::sd_log_labels(const char *variable, const char *label1, const char *label2, const char *label3, const char *label4, const char *label5)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    char buffer[64];
    strcpy(buffer, "/");
    strcat(buffer, variable);
    strcat(buffer, ".txt");

    fr = f_open(&fil, buffer, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
        DEBUG_printf("Error opening file %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    if (f_size(&fil) > 0)
    {
        DEBUG_printf("No Labels required for %s\n", buffer);
        f_close(&fil);
        return;
    }

    f_printf(&fil, "-;%s;", label1);

    if (label2 != NULL)
    {
        f_printf(&fil, "%s;", label2);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (label3 != NULL)
    {
        f_printf(&fil, "%s;", label3);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (label4 != NULL)
    {
        f_printf(&fil, "%s;", label4);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (label5 != NULL)
    {
        f_printf(&fil, "%s\n", label5);
    }
    else
    {
        f_printf(&fil, "-\n");
    }

    f_close(&fil);

    f_unmount("");
}

void SDManager::log_value(const char *variable, unsigned long time, float v1)
{
    log_values(variable, time, &v1, NULL, NULL, NULL, NULL);
}

void SDManager::log_value(const char *variable, unsigned long time, float v1, float v2)
{
    log_values(variable, time, &v1, &v2, NULL, NULL, NULL);
}

void SDManager::log_value(const char *variable, unsigned long time, float v1, float v2, float v3)
{
    log_values(variable, time, &v1, &v2, &v3, NULL, NULL);
}

void SDManager::log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4)
{
    log_values(variable, time, &v1, &v2, &v3, &v4, NULL);
}

void SDManager::log_value(const char *variable, unsigned long time, float v1, float v2, float v3, float v4, float v5)
{
    log_values(variable, time, &v1, &v2, &v3, &v4, &v5);
}

void SDManager::log_values(const char *variable, unsigned long time, float *v1, float *v2, float *v3, float *v4, float *v5)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    char buffer[64];
    strcpy(buffer, "/");
    strcat(buffer, variable);
    strcat(buffer, ".txt");

    fr = f_open(&fil, buffer, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
        DEBUG_printf("Error opening file %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    f_printf(&fil, "%lu;%f;", time, *v1);

    if (v2 != NULL)
    {
        f_printf(&fil, "%f;", *v2);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (v3 != NULL)
    {
        f_printf(&fil, "%f;", *v3);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (v4 != NULL)
    {
        f_printf(&fil, "%f;", *v4);
    }
    else
    {
        f_printf(&fil, "-;");
    }

    if (v5 != NULL)
    {
        f_printf(&fil, "%f\n", *v5);
    }
    else
    {
        f_printf(&fil, "-\n");
    }

    f_close(&fil);

    f_unmount("");
}

FSIZE_t SDManager::sd_log_size(const char *variable)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }

    char buffer[64];
    strcpy(buffer, "/");
    strcat(buffer, variable);
    strcat(buffer, ".txt");

    fr = f_open(&fil, buffer, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK)
    {
        DEBUG_printf("Error opening file %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }
    FSIZE_t size = f_size(&fil);

    f_close(&fil);
    f_unmount("");

    return size;
}

void SDManager::sd_purge_data(const char *variable)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    char buffer[64];
    strcpy(buffer, "/");
    strcat(buffer, variable);
    strcat(buffer, ".txt");

    fr = f_unlink(buffer);
    if (fr != FR_OK)
    {
        DEBUG_printf("Error deleting : %s (%d)\n", FRESULT_str(fr), fr);
    }

    f_unmount("");
}

void SDManager::sd_send_log_data(const char *value)
{
    FATFS fs;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    FRESULT res;
    FIL fil;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK)
    {
        DEBUG_printf("Device not mounted - error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    char buffer[128];
    strcpy(buffer, "/");
    strcat(buffer, value);
    strcat(buffer, ".txt");

    fr = f_open(&fil, buffer, FA_OPEN_EXISTING | FA_READ);
    if (fr != FR_OK)
    {
        DEBUG_printf("Error opening file : %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    char line[128];

    while (!f_eof(&fil))
    {
        f_gets(line, 128, &fil);

        DEBUG_printf("%s\n", line);

        pico->write_message(value, line);
    }

    pico->write_message(value, "");
    f_close(&fil);
    f_unmount("");
}