void swap(float* a, float* b) { 
    float t = *a; 
    *a = *b; 
    *b = t; 
} 
  
int partition (float* arr1, float* arr2, uint16_t low, uint16_t high) { 
    float pivot = arr[high]; 
    uint16_t i = (low - 1); 
    for (uint16_t j = low; j <= high - 1; j++) {  
        if (arr1[j] < pivot) { 
            swap(&(arr1[i]), &(arr1[j]));
            swap(&(arr2[i]), &(arr2[j]));  
        } 
    } 
    swap(&(arr1[i + 1]), &(arr1[high])); 
    swap(&(arr2[i + 1]), &(arr2[high])); 
    return (i + 1); 
} 

void quickSort(float* arr1, float* arr2, uint16_t low, uint16_t high) { 
    if (low < high) { 
        uint16_t pi = partition(arr1, arr2, low, high); 
        quickSort(arr1, arr2, low, pi - 1); 
        quickSort(arr1, arr2, pi + 1, high); 
    } 
} 