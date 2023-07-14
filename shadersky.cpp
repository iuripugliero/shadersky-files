
/////////////////////////// SHADER SKY ///////////////////////////////////

void ShaderSky::_radiance_changed() {
	if (panorama.is_valid()) {
		static const int size[RADIANCE_SIZE_MAX] = {
			32, 64, 128, 256, 512, 1024, 2048
		};
		VS::get_singleton()->sky_set_texture(sky, panorama->get_rid(), size[get_radiance_size()]);
	}
}

void ShaderSky::_queue_update(){
	if (update_queued){
		return;
	}
	update_queued = true;
	call_deferred("_update_sky");

}

void ShaderSky::_update_sky(){
	panorama = _generate_shader_sky();
	_radiance_changed();
}

Ref<Texture> ShaderSky::_generate_shader_sky(){
	update_queued = false;

	PoolVector<uint8_t> imgdata;

	static const int size[TEXTURE_SIZE_MAX] = {
		256, 512, 1024, 2048, 4096
	};

	int w = size[texture_size];
	int h = w / 2;

	imgdata.resize(w * h * 4); //RGBE
	{
		PoolVector<uint8_t>::Write dataw = imgdata.write();

		uint32_t *ptr = (uint32_t *)dataw.ptr();
        
        // UNTIL HERE FOLLOWING THE PROCEDURAL SKY IMAGE GENERATION,

		Color t_color;
		bool c_sky = sky_material->_get("COLOR", t_color);	//AS STATED IN DOCS, RETURNS TRUE IF SHADER HAVE "COLOR" PARAM
        
        
		if (c_sky){	// NEED TO GET THE OUTPUT OF SHADER SPLIT INTO TEXTURE_SIZE
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					// sky_material->_get("COLOR", t_color); //THIS FUNCTION NEEDS TO USE UV COORDINATES BASED ON I AND J INDEXES
					ptr[j * w + i] = t_color.to_rgbe9995();
				}
			}	
		
		}
		else{	//OTHERWISE PAINT IT BLACK
			for (int i = 0; i < w; i++) {
				for (int j = 0; j < h; j++) {
					t_color = Color(0, 0, 0);
					ptr[j * w + i] = t_color.to_rgbe9995();
				}
			}
		}
		
	}
	
	Ref<Image> image;
	image.instance();
	image->create(w, h, false, Image::FORMAT_RGBE9995, imgdata);

	Ref<Texture> sky_texture;
	
	VS::get_singleton()->texture_allocate(sky_texture, image->get_width(), image->get_height(), 0, Image::FORMAT_RGBE9995, VS::TEXTURE_TYPE_2D, VS::TEXTURE_FLAG_FILTER | VS::TEXTURE_FLAG_REPEAT);
	VS::get_singleton()->texture_set_data(sky_texture, image);
		
	return sky_texture;

}

void ShaderSky::set_sky_material(const Ref<ShaderMaterial> &p_sky_material){
	sky_material = p_sky_material;
	if (sky_material.is_valid()) {
		_update_sky();
		_radiance_changed();
	} else {
		VS::get_singleton()->sky_set_texture(sky, RID(), 0);
	}
}

Ref<ShaderMaterial> ShaderSky::get_sky_material() const{
	return sky_material;
}

Ref<Texture> ShaderSky::get_sky() const{
	return panorama;
}

RID ShaderSky::get_rid() const {
	return sky;
}

void ShaderSky::_bind_methods(){
	ClassDB::bind_method(D_METHOD("_update_sky"), &ShaderSky::_update_sky);
	ClassDB::bind_method(D_METHOD("set_sky_material", "material"), &ShaderSky::set_sky_material);
	ClassDB::bind_method(D_METHOD("get_sky_material"), &ShaderSky::get_sky_material);
	ClassDB::bind_method(D_METHOD("get_sky"), &ShaderSky::get_sky);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sky_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_sky_material", "get_sky_material");
}

ShaderSky::ShaderSky(){
	sky = VS::get_singleton()->sky_create();
}

ShaderSky::~ShaderSky(){
	VS::get_singleton()->free(sky);
}