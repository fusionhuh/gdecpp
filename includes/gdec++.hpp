#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

typedef class std::vector<uint8_t> vec8_t;
typedef class std::vector<std::vector<struct color>> color_frame_t;

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct frame {
    // Each frame receives its values from the corresponding image descriptor
    uint16_t img_left, img_top;
    uint16_t img_w, img_h;
    bool local_table, interlace, sort;
    uint16_t local_table_size;

    size_t data_start_index; // points to the location in the gif file at which image data actually starts

    frame();
};

struct GIF {
    private:
    FILE* fgif;

    public:
    // Logical screen descriptor fields
    uint16_t canv_w, canv_h;
    bool global_table;
    color table[256]; // allocating table on stack to avoid dynamic allocation
    uint16_t color_res;
    size_t global_table_pointer;
    bool sort; // archaic?
    uint8_t back_col_index; // generally unused

    // Graphics control extension fields
    uint8_t disposal_method;
    bool user_input, transparency;
    uint16_t delay_time;
    uint8_t transparent_col_index;

    std::vector<frame> frames;

    GIF();

    GIF(FILE* fgif);

    color col_at_index(size_t frame, size_t row, size_t column);

    std::vector<color> row_of_colors(size_t frame, size_t row);
    std::vector<color> column_of_colors(size_t frame, size_t column);

    std::vector<std::vector<color>> frame_colors(size_t frame_num);
};



/// DEFINITION PART (for API)



