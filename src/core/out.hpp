
#ifndef OUT_H
#define OUT_H

namespace out {
	
	class Head {
		
	public:
		template <T>
		std::ostream& operator << (std::ostream& os, const T& obj)
		{
			
			return os;
			
		}
		
	private:
		const char* mp_tag;
		
	};
	
	class Child {
		
	};
	
};

#endif // OUT_H

