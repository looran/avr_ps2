/* 
 * 2018, Laurent Ghigonis <laurent@gouloum.fr>
 */

void ps2_init(void);
void ps2_read(void);
void ps2_buf_push(unsigned char c);
unsigned char ps2_buf_pull(void);
void ps2_buf_empty(void);
