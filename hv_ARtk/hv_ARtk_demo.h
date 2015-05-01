
#if !defined(__HV_ARTK_DEMO_H__INCLUDED_)
#define __HV_ARTK_DEMO_H__INCLUDED_


#define HV_PTR_SHOW true
#define HV_PTR_NO_SHOW false
#define HV_PTR_LBUTTON_PRESSED 1
#define HV_PTR_RBUTTON_PRESSED 2
#define HV_PTR_BUTTON_RELEASED 3
#define HV_PTR_NO_BUTTON_ACTION 4



/*
 * triplet of doubles, for positions, colors
 */
typedef struct
{
	double	x;
	double	y;
	double	z;
} d3;

void hv_init(const char* conductor_fname, 
	     const char* save_fname_root,
	     int width, int height);
void hv_pointer_update (bool show, int button, int x, int y );
void hv_process_frame(char* pData, int width, int height);
void hv_save_frame(char* pData, int width, int height, bool fullimg);
void hv_draw(bool rendering);
void hv_show_message(const char* msg);
void hv_key_pressed(char c);
void hv_set_anchor_transform(bool visible, double m[16]);
void hv_set_floater_transform(int id, bool visible, double m[16]);
void hv_toggle_adjust_exposure();


#endif // __HV_ARTK_DEMO_H__INCLUDED_
