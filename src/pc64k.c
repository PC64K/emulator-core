#include <pc64k.h>

PC64K* pc64k_alloc_init(uint8_t* rom, size_t rom_size,
    PC64KDiskSizeGetter cb_disk_size,
    PC64KDiskReader cb_disk_read,
    PC64KDiskWriter cb_disk_write,
    PC64KMicrosGetter cb_micros) {
    if(rom_size > 65535)
        return NULL;
    PC64K* ctx = (PC64K*) malloc(sizeof(PC64K));
    memcpy(ctx->ram, rom, rom_size);

    ctx->cb_disk_size = cb_disk_size;
    ctx->cb_disk_read = cb_disk_read;
    ctx->cb_disk_write = cb_disk_write;
    ctx->cb_micros = cb_micros;

    ctx->stack_pos = 0;
    ctx->pc = 0;

    pc64k_video_init(&ctx->video);
    memset(ctx->reg, 0, sizeof(ctx->reg));

    return ctx;
}
void pc64k_deinit_free(PC64K* ctx) {
    free(ctx);
}

typedef struct {
    uint8_t x;
    uint8_t y;
} PC64KFourBitPair;

static uint8_t read_char(PC64K* ctx) {
    return ctx->ram[ctx->pc++];
}
static uint16_t read_word(PC64K* ctx) {
    uint16_t w = (((uint16_t) ctx->ram[ctx->pc]) << 8) | ctx->ram[ctx->pc + 1];
    ctx->pc += 2;
    return w;
}
static uint16_t read_word_ram(PC64K* ctx, uint16_t index) {
    return (((uint16_t) ctx->ram[index]) << 8) | ctx->ram[index + 1];
}
static PC64KFourBitPair read_four_bit_pair(PC64K* ctx) {
    uint8_t c = read_char(ctx);
    return (PC64KFourBitPair) {
        .x = (c >> 4) & 0xf,
        .y = c & 0xf
    };
}

#define COMPARE_XYZZZZ(sign) PC64KFourBitPair pair = read_four_bit_pair(ctx); \
    uint16_t addr = read_word(ctx); \
    if(ctx->reg[pair.x] sign ctx->reg[pair.y]) ctx->pc = addr;
#define COMPARE_XNYYZZZZ(sign) if(ctx->reg[pair.x] sign val) ctx->pc = addr;

void pc64k_setkey(PC64K* ctx, uint8_t key, bool down) {
    if(down) ctx->keyboard[(key >> 3) & 0b11111] |= (1 << (key & 0b111));
    else ctx->keyboard[(key >> 3) & 0b11111] &= ~(1 << (key & 0b111));
}
static bool pc64k_getkey(PC64K* ctx, uint8_t key) {
    return (ctx->keyboard[(key >> 3) & 0b11111] >> (key & 0b111)) & 1;
}
void pc64k_tick(PC64K* ctx) {
    uint8_t opcode = read_char(ctx);
    if(opcode == 0x00)
        ctx->pc = read_word(ctx);
    if(opcode == 0x01) {
        uint16_t addr = read_word(ctx);
        ctx->pc = read_word_ram(ctx, addr);
    } else if(opcode == 0x02) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->pc = (ctx->reg[pair.x] << 8) | ctx->reg[pair.y];
    } else if(opcode == 0x03) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.y] = ctx->reg[pair.x];
    } else if(opcode == 0x04) {
        uint16_t addr = read_word(ctx);
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        if(pair.y == 0x0)
            ctx->ram[addr] = ctx->reg[pair.x];
        else if(pair.y == 0x1)
            ctx->reg[pair.x] = ctx->ram[addr];
        else if(pair.y == 0x2)
            ctx->cb_disk_write(addr, &ctx->reg[pair.x], 1);
        else if(pair.y == 0x3)
            ctx->cb_disk_read(addr, &ctx->reg[pair.x], 1);
    } else if(opcode == 0x05) {
        uint16_t addr = read_word(ctx);
        size_t disk_size = ctx->cb_disk_size();
        ctx->ram[addr + 0] = (disk_size >> 16) & 0xff;
        ctx->ram[addr + 1] = (disk_size >>  8) & 0xff;
        ctx->ram[addr + 2] = (disk_size >>  0) & 0xff;
    } else if(opcode == 0x06) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        uint8_t val = read_char(ctx);
        if(pair.x == 0x0)
            ctx->reg[pair.y] = val;
        else if(pair.x == 0x1) {
            int16_t res = (int16_t) ctx->reg[pair.y] + val;
            ctx->reg[0xf] = res > 0xff ? 1 : 0;
            ctx->reg[pair.y] = (uint8_t) res;
        } else if(pair.x == 0x2) {
            int16_t res = (int16_t) ctx->reg[pair.y] - val;
            ctx->reg[0xf] = res < 0 ? 1 : 0;
            ctx->reg[pair.y] = (uint8_t) res;
        } else if(pair.x == 0x3) {
            int16_t res = (int16_t) val - ctx->reg[pair.y];
            ctx->reg[0xf] = res < 0 ? 1 : 0;
            ctx->reg[pair.y] = (uint8_t) res;
        } else if(pair.x == 0x4) {
            int16_t res = (int16_t) ctx->reg[pair.y] * val;
            ctx->reg[0xf] = res > 0xff ? 1 : 0;
            ctx->reg[pair.y] = (uint8_t) res;
        } else if(pair.x == 0x5)
            ctx->reg[pair.y] |= val;
        else if(pair.x == 0x6)
            ctx->reg[pair.y] &= val;
        else if(pair.x == 0x7)
            ctx->reg[pair.y] ^= val;
        else if(pair.x == 0x8)
            ctx->reg[pair.y] >>= val;
        else if(pair.x == 0x9)
            ctx->reg[pair.y] <<= val;
    } else if(opcode == 0x07) { // TODO: make macros maybe?
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        int16_t res = (int16_t) ctx->reg[pair.x] + ctx->reg[pair.y];
        ctx->reg[0xf] = res > 0xff ? 1 : 0;
        ctx->reg[pair.x] = res;
    } else if(opcode == 0x08) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        int16_t res = (int16_t) ctx->reg[pair.x] - ctx->reg[pair.y];
        ctx->reg[0xf] = res < 0 ? 1 : 0;
        ctx->reg[pair.x] = res;
    } else if(opcode == 0x09) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        int16_t res = (int16_t) ctx->reg[pair.y] - ctx->reg[pair.x];
        ctx->reg[0xf] = res < 0 ? 1 : 0;
        ctx->reg[pair.x] = res;
    } else if(opcode == 0x0a) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        int16_t res = (int16_t) ctx->reg[pair.x] * ctx->reg[pair.y];
        ctx->reg[0xf] = res > 0xff ? 1 : 0;
        ctx->reg[pair.x] = res;
    } else if(opcode == 0x0b) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.x] |= ctx->reg[pair.y];
    } else if(opcode == 0x0c) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.x] &= ctx->reg[pair.y];
    } else if(opcode == 0x0d) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.x] ^= ctx->reg[pair.y];
    } else if(opcode == 0x0e) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.x] >>= ctx->reg[pair.y];
    } else if(opcode == 0x0f) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->reg[pair.x] <<= ctx->reg[pair.y];
    } else if(opcode == 0x10)
        ctx->pc = ctx->stack[--ctx->stack_pos];
    else if(opcode == 0x11) {
        ctx->stack[ctx->stack_pos++] = ctx->pc;
        ctx->pc = read_word(ctx);
    } else if(opcode == 0x12) {
        uint16_t val = ctx->stack[--ctx->stack_pos];
        uint16_t addr = read_word(ctx);
        ctx->ram[addr + 0] = (val >> 8) & 0xff;
        ctx->ram[addr + 1] = (val >> 0) & 0xff;
    } else if(opcode == 0x13) {
        uint16_t addr = read_word(ctx);
        ctx->stack[ctx->stack_pos++] = (ctx->ram[addr + 0] << 8) | ctx->ram[addr + 1];
    } else if(opcode == 0x14) {
        COMPARE_XYZZZZ(==);
    } else if(opcode == 0x15) {
        COMPARE_XYZZZZ(!=);
    } else if(opcode == 0x16) {
        COMPARE_XYZZZZ(>);
    } else if(opcode == 0x17) {
        COMPARE_XYZZZZ(<);
    } else if(opcode == 0x18) {
        COMPARE_XYZZZZ(>=);
    } else if(opcode == 0x19) {
        COMPARE_XYZZZZ(<=);
    } else if(opcode == 0x1a) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        uint8_t val = read_char(ctx);
        uint16_t addr = read_word(ctx);
        if(pair.y == 0x0) { COMPARE_XNYYZZZZ(==) }
        if(pair.y == 0x1) { COMPARE_XNYYZZZZ(!=) }
        if(pair.y == 0x2) { COMPARE_XNYYZZZZ(>) }
        if(pair.y == 0x3) { COMPARE_XNYYZZZZ(<) }
        if(pair.y == 0x4) { COMPARE_XNYYZZZZ(>=) }
        if(pair.y == 0x5) { COMPARE_XNYYZZZZ(<=) }
    } else if(opcode == 0x1c) // TODO: timers
        pc64k_video_print(&ctx->video, (PC64KCharacter) {
            .font = 0,
            .character = read_char(ctx)
        });
    else if(opcode == 0x1d)
        pc64k_video_print(&ctx->video, (PC64KCharacter) {
            .font = 1,
            .character = read_char(ctx)
        });
    else if(opcode == 0x1e) {
        uint8_t index = read_char(ctx);
        memcpy(ctx->video.custom_font[index], &ctx->ram[read_word(ctx)], sizeof(ctx->video.custom_font[0]));
    } else if(opcode == 0x1f) {
        uint8_t key = read_char(ctx);
        uint16_t addr = read_word(ctx);
        if(pc64k_getkey(ctx, key)) ctx->pc = addr;
    } else if(opcode == 0x20) { // TODO: keyboard
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->video.fg_color = ctx->reg[pair.x];
        ctx->video.bg_color = ctx->reg[pair.y];
        pc64k_redraw(&ctx->video);
    } else if(opcode == 0x21)
        pc64k_video_clear(&ctx->video);
    else if(opcode == 0x22) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        ctx->video.character_x = ctx->reg[pair.x];
        ctx->video.character_y = ctx->reg[pair.y];
    } else if(opcode == 0x23) {
        uint16_t from_addr = read_word(ctx);
        uint16_t to_addr = read_word(ctx);
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        if(pair.y == 0x0) ctx->cb_disk_read(from_addr, ctx->ram + to_addr, ctx->reg[pair.x]);
        else if(pair.y == 0x1) ctx->cb_disk_write(to_addr, ctx->ram + from_addr, ctx->reg[pair.x]);
    } else if(opcode == 0x24) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        pc64k_video_print(&ctx->video, (PC64KCharacter) {
            .font = pair.x == 0x0 ? 0 : 1,
            .character = ctx->reg[pair.y]
        });
    } else if(opcode == 0x25) {
        PC64KFourBitPair pair = read_four_bit_pair(ctx);
        uint16_t addr = read_word(ctx);
        if(pc64k_getkey(ctx, ctx->reg[pair.y])) ctx->pc = addr;
    } 
}