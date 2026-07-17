/*
 * max7219.c
 *
 *  Created on: 14-Jun-2026
 *      Author: HP
 */
#include "max7219.h"
#include "spi.h"


const uint8_t font_8x8_digits[10][8] =
{
    // 0
    {0x00, 0x3C, 0x66, 0x66, 0x6E, 0x76, 0x66, 0x3C},

    // 1
    {0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x1C, 0x18},

    // 2
    {0x00, 0x7E, 0x0C, 0x18, 0x30, 0x60, 0x66, 0x3C},

    // 3
    {0x00, 0x3C, 0x66, 0x60, 0x38, 0x60, 0x66, 0x3C},

    // 4
    {0x00, 0x30, 0x30, 0x7E, 0x36, 0x3C, 0x38, 0x30},

    // 5
    {0x00, 0x3C, 0x66, 0x60, 0x60, 0x3E, 0x06, 0x7E},

    // 6
    {0x00, 0x3C, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C},

    // 7
    {0x00, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x60, 0x7E},

    // 8
    {0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C},

    // 9
    {0x00, 0x3C, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C}
};

const uint8_t font_8x8_alpha[26][8] =
{
    // A
    {0x00, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x3C, 0x18},

    // B
    {0x00, 0x3E, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3E},

    // C
    {0x00, 0x3C, 0x66, 0x06, 0x06, 0x06, 0x66, 0x3C},

    // D
    {0x00, 0x1E, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1E},

    // E
    {0x00, 0x7E, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x7E},

    // F
    {0x00, 0x06, 0x06, 0x06, 0x3E, 0x06, 0x06, 0x7E},

    // G
    {0x00, 0x3C, 0x66, 0x66, 0x76, 0x06, 0x66, 0x3C},

    // H
    {0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66},

    // I
    {0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E},

    // J
    {0x00, 0x1C, 0x36, 0x30, 0x30, 0x30, 0x30, 0x78},

    // K
    {0x00, 0x66, 0x36, 0x1E, 0x0E, 0x1E, 0x36, 0x66},

    // L
    {0x00, 0x7E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},

    // M
    {0x00, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6},

    // N
    {0x00, 0x66, 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66},

    // O
    {0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},

    // P
    {0x00, 0x06, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E},

    // Q
    {0x00, 0x6C, 0x36, 0x56, 0x66, 0x66, 0x66, 0x3C},

    // R
    {0x00, 0x66, 0x36, 0x1E, 0x3E, 0x66, 0x66, 0x3E},

    // S
    {0x00, 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C},

    // T
    {0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E},

    // U
    {0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66},

    // V
    {0x00, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66},

    // W
    {0x00, 0xC6, 0xEE, 0xFE, 0xD6, 0xC6, 0xC6, 0xC6},

    // X
    {0x00, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66},

    // Y
    {0x00, 0x18, 0x18, 0x18, 0x3C, 0x66, 0x66, 0x66},

    // Z
    {0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E}
};

const uint8_t font_space[8] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

void MAX7219_Write(uint8_t address, uint8_t data)
{
	MAX7219_CS_LOW();

	SPI1_TransferByte(address);
	SPI1_TransferByte(data);

	MAX7219_CS_HIGH();

}

void MAX7219_Init(void)
{
	MAX7219_Write(MAX7219_REG_DISPLAY_TEST, 0x00);   // Display test off
    MAX7219_Write(MAX7219_REG_SHUTDOWN, 0x01);   // Normal operation, exit shutdown
    MAX7219_Write(MAX7219_REG_DECODE_MODE, 0x00);   // No-decode mode for digits 0-7
    MAX7219_Write(MAX7219_REG_INTENSITY, 0x08);   // Medium brightness
    MAX7219_Write(MAX7219_REG_SCAN_LIMIT, 0x07);   // Scan digits 0-7

    MAX7219_Clear();
}

void MAX7219_Clear(void)
{
    for (uint8_t i = 1; i <= 8; i++)
    {
        MAX7219_Write(i, 0x00);
    }
}

void MAX7219_Fill(void)
{
    for (uint8_t i = 1; i <= 8; i++)
    {
        MAX7219_Write(i, 0xFF);
    }
}

void MAX7219_NumberSequenceTest(void)
{
  for(uint8_t i=0; i<10; i++)
  {
	for(uint8_t j=0; j<=7; j++)
	{
		MAX7219_Write(j+1, font_8x8_digits[i][j]);
	}
	HAL_Delay(2000);
  }

}

void MAX7219_AlphabetSequenceTest(void)
{
  for(uint8_t i=0; i<26; i++)
  {
	for(uint8_t j=0; j<=7; j++)
	{
		MAX7219_Write(j+1, font_8x8_alpha[i][j]);
	}
	HAL_Delay(2000);
  }

}

void MAX7219_CharBlinking(char ch)
{
	MAX7219_DisplayChar(ch);
	HAL_Delay(500);
	MAX7219_Clear();
	HAL_Delay(500);

}

void MAX7219_StringBlinking(const char *ch)
{
    while (*ch != '\0')
    {
        MAX7219_DisplayChar(*ch);
        HAL_Delay(500);

        MAX7219_Clear();
        HAL_Delay(500);

        ch++;
    }
}

void MAX7219_DisplayNumber(uint8_t number)
{
    if (number > 9)
    {
        return;
    }

    for (uint8_t row = 0; row < 8; row++)
    {
        MAX7219_Write(row + 1, font_8x8_digits[number][row]);
    }
}

void MAX7219_CheckPosition(void)
{
    MAX7219_Clear();

    MAX7219_Write(0x01, 0x81);   // Two edge pixels on first row
    MAX7219_Write(0x08, 0x81);   // Two edge pixels on last row
}

void MAX7219_DisplayChar(char ch)
{
    const uint8_t *bitmap = MAX7219_GetCharBitmap(ch);

    for (uint8_t row = 0; row < 8; row++)
    {
        MAX7219_Write(row + 1, bitmap[row]);
    }
}

const uint8_t* MAX7219_GetCharBitmap(char ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        ch = ch - 'a' + 'A';
    }

    if (ch >= 'A' && ch <= 'Z')
    {
        return font_8x8_alpha[ch - 'A'];
    }

    if (ch >= '0' && ch <= '9')
    {
        return font_8x8_digits[ch - '0'];
    }

    return font_space;
}

void MAX7219_DisplayFrame(uint8_t frame[8])
{
    for (uint8_t row = 0; row < 8; row++)
    {
        MAX7219_Write(row + 1, frame[row]);
    }
}

uint16_t MAX7219_StrLen(const char *str)
{
    uint16_t len = 0;

    while (str[len] != '\0')
    {
        len++;
    }

    return len;
}

void MAX7219_ScrollText_RtoL(const char *text, uint16_t speed_ms)
{
    uint8_t frame[8];

    uint16_t text_len = MAX7219_StrLen(text);

    const uint8_t char_width = 8;
    const uint8_t spacing = 1;
    const uint8_t char_step = char_width + spacing;

    uint16_t total_columns = (text_len * char_step) + 8;

    for (uint16_t offset = 0; offset < total_columns; offset++)
    {
        for (uint8_t row = 0; row < 8; row++)
        {
            frame[row] = 0x00;
        }

        for (uint8_t display_col = 0; display_col < 8; display_col++)
        {
            int16_t virtual_col = (int16_t)offset + display_col - 8;

            if (virtual_col < 0)
            {
                continue;
            }

            uint16_t char_index = virtual_col / char_step;
            uint8_t col_in_char = virtual_col % char_step;

            if (char_index >= text_len)
            {
                continue;
            }

            if (col_in_char >= 8)
            {
                continue;   // spacing column
            }

            const uint8_t *bitmap = MAX7219_GetCharBitmap(text[char_index]);

            /*
             * IMPORTANT:
             * Your corrected font uses bit0 as the left-side column.
             */
            uint8_t source_mask = (uint8_t)(1U << col_in_char);
            uint8_t dest_mask   = (uint8_t)(1U << display_col);

            for (uint8_t row = 0; row < 8; row++)
            {
                if (bitmap[row] & source_mask)
                {
                    frame[row] |= dest_mask;
                }
            }
        }

        MAX7219_DisplayFrame(frame);
        HAL_Delay(speed_ms);
    }
}
