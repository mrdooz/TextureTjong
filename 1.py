import texture_ext as t
def test():
	red = t.Color(1,0,0,0)
	black = t.Color(0,0,0,0)
	yellow = t.Color(1,0.5,0,0)
	a = t.Texture(512, 512)
	big_red = t.Texture(640, 480)
	big_red2 = t.Texture(640, 480)
	big_green = t.Texture(640, 480)
	t.single_color(red, big_red)
	t.single_color(yellow, big_green)
	t.single_color(t.Color(1,1,0.5,0), big_red2)
	t.mixer(big_red, big_green, 0, a)
	for i in xrange(0, 100):
		a.set_pixel(i, i, black)

	a2 = t.Texture(512, 512)	
	o = t.Texture(512, 512)
	t.radial(1, a2)
	t.displace(a2, a2, 1, o)

	o2 = t.Texture(512, 512)
	t.remap(o, t.Color(0.0, 0.1, 0.0, 1), t.Color(0.7, 0.8, 0.3, 1), o2)

	t.save_bitmap("tjong.bmp", o2)

red = t.Color(0.5,0,0,0)
yellow = t.Color(1,1,0,0)
	
o = t.Texture(512, 512)
t.noise(0.45, 0.45, o)

o2 = t.Texture(512, 512)
t.remap(o, red, yellow, o2)

t2 = t.Texture(512, 512)
t.radial(100, 200, t2)

t3 = t.Texture(512, 512)
t.modulate(o2, t2, t3)


#g = t.Texture(512, 512)
#t.noise(0.40, 0.40, g)

#b = t.Texture(512, 512)
#t.noise(0.35, 0.35, b)

#o = t.Texture(512, 512)
#t.combine(r, g, b, o)
