struct file_data {
	void *data;
	size_t len;
};

struct file_data *load_file(char *path);
