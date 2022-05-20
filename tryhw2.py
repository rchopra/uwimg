from uwimg import *

im = load_image("data/dog.jpg")
f = make_box_filter(7)
blur = convolve_image(im, f, 1)
save_image(blur, "dog-box7")

thumb = nn_resize(blur, blur.w // 7, blur.h // 7)
save_image(thumb, "dogthumb")

f = make_highpass_filter()
blur = convolve_image(im, f, 0)
clamp_image(blur)
save_image(blur, "dog-highpass")

f = make_sharpen_filter()
blur = convolve_image(im, f, 1)
clamp_image(blur)
save_image(blur, "dog-sharpen")

f = make_emboss_filter()
blur = convolve_image(im, f, 1)
clamp_image(blur)
save_image(blur, "dog-emboss")
