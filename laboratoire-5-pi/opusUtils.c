#include "includes.h"

// Fonction pour placer le fichier en mémoire
uchar *map_file_encode(const char *fn)
{
    FILE *fp = fopen(fn, "r");
    if (!fp)
    {
        printf("fail to open file: %s", fn);
        return (uchar *)MAP_FAILED;
    }

    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    uchar *ptr = (uchar *)mmap(NULL, pos, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);

    fclose(fp);
    return ptr;
}

uchar *map_file_decode(const char *fn, size_t *size)
{
    FILE *fp = fopen(fn, "r");
    if (!fp)
    {
        printf("fail to open file: %s", fn);
        return (uchar *)MAP_FAILED;
    }

    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    uchar *ptr = (uchar *)mmap(NULL, pos, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);
    *size = pos;
    fclose(fp);
    return ptr;
}

int decode(const char *audioFile)
{
    int ret;
    size_t file_size = 0;

    uchar *input_ptr = map_file_decode(audioFile, &file_size); // Pointeur de début de fichier
    if (input_ptr == MAP_FAILED)
    {
        printf("fail to map file");
        return -1;
    }
    uchar *reading_end = input_ptr; // Pointeur actif

    // On récupère le header wav
    wavfile_header_t *header = (wavfile_header_t *)input_ptr;
    // Donnés utile pour le décodage
    int32_t sample_rate = header->SampleRate;
    int16_t n_channels = header->NumChannels;
    int32_t wav_header_size = sizeof(wavfile_header_t);

    // Initialise le décodeur
    OpusDecoder *decoder = NULL;
    int size = 0;
    size = opus_decoder_get_size(n_channels); /* max 2 channels */
    decoder = (OpusDecoder *)malloc(size);
    if (!decoder)
    {
        printf("fail to create encoder");
        return -1;
    }

    ret = opus_decoder_init(decoder, sample_rate, n_channels);
    if (ret != 0)
    {
        printf("fail to init %d", ret);
        return ret;
    }

    // Fichier de sortie qui va contenir le signal décodé (WAV).
    FILE *fp = fopen("decoded.wav", "w");
    if (!fp)
    {
        printf("fail to open file decoded.wav");
        return -1;
    }
    // On sauvegarde le header wav.
    fwrite(header, 1, wav_header_size, fp);
    reading_end += wav_header_size; // On skip le header pour débuter l'encodage
    opus_int32 chunk_size = CHUNK;
    int frame_duration_ms = 5;
    int frame_size = sample_rate / 1000 * frame_duration_ms;
    opus_int16 *buffer = (opus_int16 *)malloc(frame_size * n_channels * sizeof(opus_int16));
    while (reading_end < input_ptr + file_size)
    {

        ret = opus_decode(decoder, reading_end, chunk_size, buffer, frame_size, 0);
        if (ret <= 0)
        {
            printf("fail to decode : %d\n", ret);
            break;
        }

        fwrite(buffer, 1, ret * 2, fp);

        reading_end += chunk_size;
    }

    fclose(fp);
    munmap(input_ptr, file_size);
    free(decoder);

    printf("enc: done\n");

    return 0;
}

int encode(const char *audioIn, const char *audioOut)
{
    int ret;

    uchar *input_ptr = map_file_encode(audioIn); // Pointeur de début de fichier
    if (input_ptr == MAP_FAILED)
    {
        printf("fail to map file");
        return -1;
    }
    int16_t *reading_end = (int16_t *)input_ptr; // Pointeur actif

    // On s'assure que le fichier est bien un fichier audio .wav
    wavfile_header_t *header = (wavfile_header_t *)input_ptr;
    char wav_marker[5] = "RIFF";
    if (strncmp(header->ChunkID, wav_marker, 4) != 0)
    {
        printf("The input file is not a wav file..");
        return -1;
    }

    // Donnés utile pour l'encodage
    int32_t sample_rate = header->SampleRate;
    printf("sample rate is : %d\n", sample_rate);
    int16_t n_channels = header->NumChannels;
    printf("n channels are : %d\n", n_channels);
    int32_t audio_size = header->Subchunk2Size;
    int32_t wav_header_size = sizeof(wavfile_header_t);
    int32_t total_file_size = wav_header_size + audio_size;

    // Initialisation de l'encodeur Opus
    OpusEncoder *encoder = NULL;
    int size = 0;
    size = opus_encoder_get_size(n_channels);
    encoder = (OpusEncoder *)malloc(size);
    if (!encoder)
    {
        printf("fail to create encoder");
        return -1;
    }
    // Ici on détermine le sample rate, le nombre de channel et un option pour la méthode d'encodage.
    // OPUS_APPLICATION_RESTRICTED_LOWDELAY nous donne la plus faible latence, mais vous pouvez essayer d'autres options.
    ret = opus_encoder_init(encoder, sample_rate, n_channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY);
    if (ret != 0)
    {
        printf("fail to init %d", ret);
        return ret;
    }

    /* paramètres de l'encodeur */
    // Bitrate du signal encodé : Vous devez trouver le meilleur compromis qualité/transmission.
    ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BIT_RATE));
    if (ret != 0)
    {
        printf("fail to set bitrate");
        return ret;
    }

    // Complexité de l'encodage, permet un compromis qualité/latence
    ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
    if (ret != 0)
    {
        printf("fail to set complexity");
        return ret;
    }

    // Variation du bitrate.
    ret = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
    if (ret != 0)
    {
        printf("fail to set vbr");
        return ret;
    }
    // type de signal à encoder (dans notre cas il s'agit de la musique)
    ret = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    if (ret != 0)
    {
        printf("fail to set signal");
        return ret;
    }

    // Fichier de sortie qui va contenir le signal encodé.
    FILE *fp = fopen(audioOut, "w");
    if (!fp)
    {
        printf("fail to open file %s", audioOut);
        return -1;
    }
    // On sauvegarde le header wav simplement pour le réutiliser lors du décodage.
    fwrite(header, 1, wav_header_size, fp);
    reading_end += wav_header_size; // On skip le header pour débuter l'encodage

    int max_data_size = 100; // Nombre maximal de byte encodé par frame
    uchar *buffer = (uchar *)malloc(max_data_size);
    int frame_duration_ms = 5; // Une durée plus grande que 10 ms va introduire énormément de bruit dans un signal musical.
    int frame_size = sample_rate / 1000 * frame_duration_ms;
    printf("frame size is : %d\n", frame_size);
    clock_t start_time = clock();
    while ((uchar *)reading_end < input_ptr + total_file_size)
    {
        ret = opus_encode(encoder, (opus_int16 *)reading_end, frame_size, buffer, max_data_size);
        if (ret <= 0)
        {
            printf("fail to encode");
            break;
        }
        // Écriture du signal encodé dans le fichier
        fwrite(buffer, 1, ret, fp);

        // Incrémente le pointeur
        reading_end += frame_size;
    }
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("encoding opus done in %f sec\n", time_spent);

    fclose(fp);
    munmap(input_ptr, total_file_size);
    free(encoder);

    return 0;
}