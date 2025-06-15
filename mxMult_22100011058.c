//TUNCAY BAYIR
// 22100011058
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 20

int **matrixA, **matrixB, **matrixC;
int matrixSize;
pthread_mutex_t mutex; // Global mutex

struct ThreadArgs {
    int row;
    int col;
};

// Matrisi dosyadan okuma fonksiyonu
void readMatrixFromFile(const char* filename, int **matrix) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Dosya açılamadı: %s\n", filename);
        exit(1);
    }
    
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            if(fscanf(file, "%d", &matrix[i][j]) != 1) {
                printf("Dosya okuma hatası\n");
                exit(1);
            }
        }
    }
    fclose(file);
}

// Sağa kaydırma işlemi
void* shiftRight(void* arg) {
    int shift = *(int*)arg;
    int **temp = malloc(matrixSize * sizeof(int*));
    for(int i = 0; i < matrixSize; i++) {
        temp[i] = malloc(matrixSize * sizeof(int));
    }
    
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            int newCol = (j + shift) % matrixSize;
            temp[i][newCol] = matrixA[i][j];
        }
    }
    
    for(int i = 0; i < matrixSize; i++) {
        memcpy(matrixA[i], temp[i], matrixSize * sizeof(int));
        free(temp[i]);
    }
    free(temp);
    
    return NULL;
}

// Yukarı kaydırma işlemi
void* shiftUp(void* arg) {
    int shift = *(int*)arg;
    int **temp = malloc(matrixSize * sizeof(int*));
    for(int i = 0; i < matrixSize; i++) {
        temp[i] = malloc(matrixSize * sizeof(int));
    }
    
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            int newRow = (i + matrixSize - shift) % matrixSize;
            temp[newRow][j] = matrixA[i][j];
        }
    }
    
    for(int i = 0; i < matrixSize; i++) {
        memcpy(matrixA[i], temp[i], matrixSize * sizeof(int));
        free(temp[i]);
    }
    free(temp);
    
    return NULL;
}

// Matris çarpımı işlemi
void* multiply(void* arg) {
    struct ThreadArgs *args = (struct ThreadArgs*)arg;
    int row = args->row;
    int col = args->col;
    
    int result = 0;
    
    // Standart matris çarpımı
    for(int k = 0; k < matrixSize; k++) {
        result += matrixA[row][k] * matrixB[k][col];
    }
    
    pthread_mutex_lock(&mutex);
    matrixC[row][col] = result;
    pthread_mutex_unlock(&mutex);
    
    free(arg);
    return NULL;
}

// Main fonksiyonunda debug çıktısı için eklenebilecek yardımcı fonksiyon
void printMatrix(int **matrix, const char* name) {
    printf("\nMatrix %s:\n", name);
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
// Matrisi dosyaya yazma fonksiyonu
void writeMatrixToFile(const char* filename, int **matrix) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Dosya açılamadı: %s\n", filename);
        exit(1);
    }
    
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Kullanım: %s inputA.txt\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "r");
    if(file == NULL) {
        printf("Dosya açılamadı\n");
        return 1;
    }
    
    int count = 0;
    int num;
    while(fscanf(file, "%d", &num) == 1) count++;
    matrixSize = (int)sqrt(count);
    fclose(file);
    
    if (matrixSize > MAX_SIZE) {
        printf("Matris boyutu 20x20'den büyük olamaz.\n");
        return 1;
    }
    
    // Matrisleri oluştur
    matrixA = malloc(matrixSize * sizeof(int*));
    matrixB = malloc(matrixSize * sizeof(int*));
    matrixC = malloc(matrixSize * sizeof(int*));
    for(int i = 0; i < matrixSize; i++) {
        matrixA[i] = malloc(matrixSize * sizeof(int));
        matrixB[i] = malloc(matrixSize * sizeof(int));
        matrixC[i] = malloc(matrixSize * sizeof(int));
    }
    
    // Orijinal matrisi oku
    readMatrixFromFile(argv[1], matrixA);
    
    // MatrisB'yi orijinal matrisin transpozesi olarak oluştur
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            matrixB[i][j] = matrixA[j][i];
        }
    }
    
    // Mutex başlatma
    pthread_mutex_init(&mutex, NULL);
    
    // Shift miktarını hesapla
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    int shift = local->tm_sec % matrixSize;
    printf("Shift miktarı: %d\n", shift);
    
    // Sağa ve yukarı kaydırma thread'leri
    pthread_t shiftRightThread, shiftUpThread;
    pthread_create(&shiftRightThread, NULL, shiftRight, &shift);
    pthread_create(&shiftUpThread, NULL, shiftUp, &shift);

    pthread_join(shiftRightThread, NULL);
    pthread_join(shiftUpThread, NULL);
    
    // B matrisini inputB.txt dosyasına yaz
    writeMatrixToFile("inputB.txt", matrixA);
    
    pthread_t **threads = malloc(matrixSize * sizeof(pthread_t*));
    for(int i = 0; i < matrixSize; i++) {
        threads[i] = malloc(matrixSize * sizeof(pthread_t));
    }
    
    // Thread'leri oluştur
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
            args->row = i;
            args->col = j;
            pthread_create(&threads[i][j], NULL, multiply, args);
        }
    }

    // Thread'leri bekle
    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
            pthread_join(threads[i][j], NULL);
        }
    }
    
    // Sonuçları yazdır ve dosyaya yaz
    printf("\nSonuç Matrisi:\n");
    for(int i = 0; i < matrixSize; i++) {
        for(int j = 0; j < matrixSize; j++) {
            printf("%d ", matrixC[i][j]);
        }
        printf("\n");
    }
    
    writeMatrixToFile("outputC.txt", matrixC);
    
    // Temizlik
    pthread_mutex_destroy(&mutex);
    for(int i = 0; i < matrixSize; i++) {
        free(matrixA[i]);
        free(matrixB[i]);
        free(matrixC[i]);
        free(threads[i]);
    }
    free(matrixA);
    free(matrixB);
    free(matrixC);
    free(threads);
    
    return 0;
}