#include <gdec++.hpp>

using namespace std;
typedef class std::vector<uint8_t> vec8_t;
typedef class std::vector<std::vector<struct color>> color_frame_t;

template <typename T>
inline T extract_int(FILE* file, bool l_endian=false, size_t pos=-1) { // begins reading at file pointer unless specified
    size_t T_size = sizeof(T);
    if (pos!=-1) {
        fseek(file, pos, SEEK_SET);
    }

    T read_val;
    fread(&read_val, T_size, 1, file);
    printf("%d\n", read_val);
    if (T_size > 1) {
        T temp = read_val;
        T result = 0;

        if (l_endian) {
            T stamp = 0xFF;
            for (int i = 0; i < T_size; i++) {
                result &= (read_val & stamp);
                if (i < T_size - 1) result <<= 8; read_val >>= 8;
            }
            return result;
        }
        else { // big endian, can just return read integer as-is
            return read_val;
        }
    }
    else return read_val;
}

void decode_lzw(vec8_t& index_stream, const vec8_t& code_stream, vector<vec8_t>& code_table, uint8_t min) {
    size_t curr_byte_pos = 0;
    uint8_t curr_bit_pos = 0;

    uint8_t first_min = min;
    uint16_t reset_code = 1 << (min - 1);
    uint16_t eoi_code = (1 << (min - 1)) + 1;

    vector<vec8_t> first_code_table = code_table;

    size_t index = 0;
    int prev_code = 0;

    while (true) {
        while (curr_bit_pos > 7) {
            curr_bit_pos-=8;
            curr_byte_pos++;
        }

        uint16_t code = 0;
        if (8 - curr_bit_pos > min) { // TESTING DIFFERENT METHOD TO OLD FUNCTION
            uint8_t byte = code_stream[curr_byte_pos];

            if (curr_bit_pos == 0) {
                byte &= (uint8_t) ((1 << min) - 1);
            }
            else {
                byte >>= curr_bit_pos;
                byte <<= curr_bit_pos;
                byte <<= (8 - (curr_bit_pos + min));
                byte >>= (curr_bit_pos + (8 - (curr_bit_pos + min)));
            }
            code = byte;
            curr_bit_pos += min;
        }
        else if ((8 - curr_bit_pos) + 8 >= min) {
            uint8_t byte_one = code_stream[curr_byte_pos];
            uint32_t byte_two = code_stream[curr_byte_pos + 1]; // not sure if this needs to be 32 bit?

            code |= byte_one >> curr_bit_pos;
            byte_two &= ((1 << (min - (8 - curr_bit_pos) + 1)) - 1);

            uint8_t first_bits_remaining = (8 - curr_bit_pos);
            uint8_t intruding_bits = min - first_bits_remaining;
            byte_two &= ((1 << intruding_bits) - 1);
            byte_two <<= first_bits_remaining;
            code|=byte_two;

            curr_bit_pos+=min;
        }
        else {
            uint8_t byte_one = code_stream[curr_byte_pos];
            uint32_t byte_two = code_stream[curr_byte_pos + 1]; // not sure if this needs to be 32 bit?
            uint32_t byte_three = code_stream[curr_byte_pos + 2];

            code |= (byte_one >> curr_bit_pos);
            byte_two &= (1 << (min - (8 - curr_bit_pos) + 1)) - 1;

            uint8_t first_bits_remaining = (8 - curr_bit_pos);
            uint8_t intruding_bits = min - 8 - first_bits_remaining;

            code |= (byte_one >> curr_bit_pos);
            code |= (byte_two << first_bits_remaining);
            byte_three &= (1 << intruding_bits) - 1;
            byte_three <<= (8 + first_bits_remaining);
            code |= byte_three;

            curr_bit_pos += min;
        }

        if (index == 1) {
            index_stream.push_back(code_table[code][0]);
        }
        else if (code == reset_code) {
            min = first_min;
            code_table = first_code_table;
            index = 0;
        }
        else if (code == eoi_code) {
            break;
        }
        else {
            vec8_t new_entry; new_entry.reserve(1 << 4);
            if (code <= code_table.size() - 1) {
                /*index_stream.insert(new_entry.end(), code_table[code].begin(), code_table[code].end());
                new_entry.insert(new_entry.end(), code_table[prev_code].begin(), code_table[prev_code].end());

                new_entry.push_back(code_table[code][0]);
                code_table.push_back(new_entry);*/

                // COME BACK HERE AND FIX INEFFICIENT CODE

                for (uint8_t i : code_table[code]) {
                    index_stream.push_back(i);
                }
                for (uint8_t i : code_table[prev_code]) {
                    new_entry.push_back(i);
                }
                new_entry.push_back(code_table[code][0]);
                new_entry.shrink_to_fit();
                code_table.push_back(new_entry);
            }
            else {
                /*auto begin = code_table[prev_code].begin(), end = code_table[prev_code].end();
                index_stream.insert(index_stream.end(), begin, end);
                new_entry.insert(new_entry.end(), begin, end);

                index_stream.push_back(code_table[prev_code][0]);
                new_entry.push_back(code_table[prev_code][0]);
                code_table.push_back(new_entry);*/
                for (uint8_t i : code_table[prev_code]) {
                    index_stream.push_back(i);

                    new_entry.push_back(i);
                }
                index_stream.push_back(code_table[prev_code][0]);
                new_entry.push_back(code_table[prev_code][0]);
                new_entry.shrink_to_fit();
                code_table.push_back(new_entry);
            }
        }
        prev_code = code;

        if ((code_table.size() - 1 == (1 << min) - 1) && min != 12) {
            min++;
        }
        index++;
    }
}


frame::frame() { }


vector<color> GIF::row_of_colors(size_t frame, size_t row) {

}

vector<color> GIF::column_of_colors(size_t frame, size_t column) {

}

color_frame_t GIF::frame_colors(size_t frame_num) {
    frame color_frame = frames[frame_num];
    size_t data_pos = color_frame.data_start_index;

    fseek(fgif, data_pos, SEEK_SET);

    if (color_frame.local_table) {
        // skip table data for now
        fseek(fgif, color_frame.local_table_size * 3, SEEK_CUR);
        printf("WARNING: LOCAL COLOR TABLE\n");
    }

    uint8_t min_size;
    fread(&min_size, 1, 1, fgif); min_size++;
    printf("min_size: %d\n", min_size);

    vector<vec8_t> base_table((1 << (min_size - 1)) + 2);
    for (int i = 0; i < base_table.size(); i++) {
        base_table[i] = vec8_t(); base_table[i].reserve(1);
        base_table[i][0] = i;
    }

    uint8_t block_size;
    fread(&block_size, 1, 1, fgif);

    size_t code_stream_size = 0;

    // get total size of code stream first time around
    size_t return_pos = ftell(fgif) - 1;
    while (block_size != 0x0) {
        code_stream_size += block_size;
        fseek(fgif, block_size, SEEK_CUR);
        fread(&block_size, 1, 1, fgif);
    }

    fseek(fgif, return_pos, SEEK_SET);
    vec8_t code_stream(code_stream_size);

    fread(&block_size, 1, 1, fgif);
    size_t bytes_read = 0;
    while (block_size != 0x0) {
        bytes_read += fread(&(code_stream[0 + bytes_read]), 1, block_size, fgif);
        fread(&block_size, 1, 1, fgif);
    }

    vec8_t index_stream; index_stream.reserve(color_frame.img_h * color_frame.img_w); // this size preallocation appears to be correct
    decode_lzw(index_stream, code_stream, base_table, min_size); // index_stream now contains all of the color indices we want

    color_frame_t color_stream(color_frame.img_h);
    for (auto i : color_stream) {
        i = vector<color>(color_frame.img_w);
    }

    if (color_frame.local_table) {
        struct color local_table[256];
        fseek(fgif, color_frame.data_start_index, SEEK_CUR);
        fread(local_table, 1, (color_frame.local_table_size * 3), fgif);
        for (int a = 0; a < color_frame.img_h; a++) {
            for (int b = 0; b < color_frame.img_w; b++) {
                color_stream[a][b] = local_table[index_stream[a * color_frame.img_h + b]];
            }
        } 
    }
    else {
        for (int a = 0; a < color_frame.img_h; a++) {
            for (int b = 0; b < color_frame.img_w; b++) {
                color_stream[a][b] = table[index_stream[a * color_frame.img_h + b]];
            }
        } 
    }
    return color_stream;
}

GIF::GIF(FILE* fgif) : fgif(fgif) {
    fseek(fgif, 0, SEEK_END);
    size_t gif_size = ftell(fgif); // determining size of file
    rewind(fgif);

    GIF* gif = new GIF(fgif);
        
    fseek(fgif, 0x6, SEEK_SET);
    canv_w = extract_int<uint16_t>(fgif, false);
    canv_h = extract_int<uint16_t>(fgif, false);
    printf("File pointer after finding canvas dims: %lu\n", ftell(fgif));
    printf("%d %d\n", canv_h, canv_w);

    uint8_t packed; // packed field
    printf("byte pos for packed field: %lu\n", ftell(fgif));
    fread(&packed, 1, 1, fgif);
    global_table = (packed & 0b10000000) ? true : false;
    color_res = ( 2 << ((packed & 0b00000111) >> 0) ); // color_res is equivalent to global table size

    fread(&(back_col_index), 1, 1, fgif);
     // next field is irrelevant so skip
    fseek(fgif, 1, SEEK_CUR);

    if (global_table) {
        fread(table, 1, color_res * 3, fgif);
    }

    vector<frame> frames((1 << 6)); // 64 frames preallocated
    while (1) {
        uint8_t descriptor;
        fread(&descriptor, 1, 1, fgif); // Should settle on byte 0x3B, should have EoF after this read

        if (feof(fgif) || descriptor == 0x3B) break;

        if (descriptor == 0x21) {
            fread(&descriptor, 1, 1, fgif);
            switch (descriptor) {
                case 0xF9: // graphics control
                    fseek(fgif, 1, SEEK_CUR); // byte size is useless for us
                    fread(&packed, 1, 1, fgif);
                    transparency = ((packed & 0b00000001) == 1) ? true : false;
                    disposal_method = ((packed & 0x1C) >> 2);
                    delay_time = extract_int<uint16_t>(fgif, true);

                    fread(&(transparent_col_index), 1, 1, fgif);
                    fread(&descriptor, 1, 1, fgif);

                    if (descriptor != 0x00) {
                        fprintf(stderr, "Something has gone wrong decoding a graphics control extension.\n");
                        exit(-1);
                    }

                    break;

                case 0x01: // plain text, add support later

                    break;

                case 0xFF: // application, don't need to do anything with this apparently
                    fread(&descriptor, 1, 1, fgif);
                    while (descriptor != 0x0) { 
                        fseek(fgif, descriptor, SEEK_CUR);
                        fread(&descriptor, 1, 1, fgif); 
                    }
                    break;

                case 0xFE: // comment
                    fread(&descriptor, 1, 1, fgif);
                    while (descriptor != 0x0) {
                        fseek(fgif, descriptor, SEEK_CUR);
                        fread(&descriptor, 1, 1, fgif);
                    }
                    break;

                default:
                    fprintf(stderr, "Unrecognized control extension found.\n");
                    exit(-1);
            }
        }
        else if (descriptor == 0x2C) { // image descriptor, time to create new frame
            frame curr_frame;

            curr_frame.img_left = extract_int<uint16_t>(fgif, false);
            curr_frame.img_top = extract_int<uint16_t>(fgif, false);

            curr_frame.img_w = extract_int<uint16_t>(fgif, false);
            curr_frame.img_h = extract_int<uint16_t>(fgif, false);
            printf("%d %d\n", curr_frame.img_w, curr_frame.img_h);

            fread(&packed, 1, 1, fgif);

            curr_frame.local_table = ((packed & 0b10000000) > 0) ? true : false;
            curr_frame.interlace = ((packed & 0b01000000) > 0) ? true : false;
            curr_frame.sort = ((packed & 0b00100000) > 0) ? true : false;

            curr_frame.data_start_index = ftell(fgif);
            if (curr_frame.local_table) {
                curr_frame.local_table_size = ( 2 << ((packed & 0b00000111) >> 0) );
                curr_frame.data_start_index = ftell(fgif);
                fseek(fgif, 3 * curr_frame.local_table_size, SEEK_CUR);
            }

            frames.push_back(curr_frame);

            fseek(fgif, 1, SEEK_CUR);
            fread(&descriptor, 1, 1, fgif);
            while (descriptor != 0) {
                fseek(fgif, descriptor, SEEK_CUR);
                fread(&descriptor, 1, 1, fgif);
            }
        }
        else {
            fprintf(stderr, "Unrecognized descriptor, something has probably gone wrong during file decoding. File pointer: %lu\n", ftell(fgif));
            exit(-1);
        }
    }
    frames.shrink_to_fit();
}

