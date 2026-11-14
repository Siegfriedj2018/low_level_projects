#include <stdio.h>
#include <kv.h>

int main() {
  kv_t *table = kv_init(10);

  printf("%p\n", table);
  printf("%ld\n", table->capacity);

  int id = kv_put(table, "hehe", "haha");
  int id2 = kv_put(table, "hehe", "hoho");
  int id3 = kv_put(table, "lala", "hoho");
  int id4 = kv_put(table, "fala", "hoho");

  printf("returned value:\n\tid1: [%d]\n\tid2: [%d]\n\tid3: [%d]\n\tid4: [%d]\n",id, id2, id3, id4);

  for (int i = 0; i < table->capacity; i++) {
    if (table->entries[i].key) {
      printf("[%d] %s: %s\n",
        i,
        table->entries[i].key,
        table->entries[i].value);
    }
  }
}
