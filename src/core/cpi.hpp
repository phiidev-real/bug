
namespace cpi {
	
	int Awake();
	void Delete();
	
	struct Omma {
		
		char* p_data;
		unsigned int length;
		
	};
	
	Omma Read(const char*);
	
};

