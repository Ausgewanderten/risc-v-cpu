extern const unsigned char rom_thwaite_nes[];
extern unsigned int rom_thwaite_nes_len;

struct rom {
  const char *name;
  const void *body;
  unsigned int *size;
};

struct rom roms[] = {
  { .name = "thwaite", .body = rom_thwaite_nes, .size = &rom_thwaite_nes_len, },
};
int nroms = 1;
