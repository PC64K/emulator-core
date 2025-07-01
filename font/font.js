import { PNG } from "pngjs";
import { readFileSync } from "fs";

const png = PNG.sync.read(readFileSync("font.png"));
let str = "uint8_t system_font[256][16] = {\n";
for(let y = 0; y < 16; y++)
    for(let x = 0; x < 16; x++) {
        str += `\t// ${y.toString(16)}${x.toString(16)}\n\t{\n`;
        for(let z = 0; z < 16; z++) {
            let row = 0;
            for(let a = 0; a < 8; a++) {
                const index = ((png.width * (y * 16 + 16 + z) + (x * 8 + 8 + a)) << 2) + 3;
                const alpha = png.data[index];
                row <<= 1;
                if(alpha > 127) row |= 1;
            }
            str += `\t\t0b${row.toString(2).padStart(8, "0")},\n`;
        }
        str += "\t},\n";
    }
str += "};";
console.log(str);