//
// Collection of useful exceptions, all of which extend std::exception
//
#ifndef USEFUL_EXCEPTIONS_HPP
#define USEFUL_EXCEPTIONS_HPP

#include <string>
#include <exception>

class fileNotFoundException : public std::exception {
	// used in ini_config, when reading out the config file
	private:
		std::string* pFilename;
		std::string* pMsg;
	public:
		fileNotFoundException(std::string* pFilename) {
			this->pFilename = pFilename;
			this->pMsg = new std::string("FATAL: file " + *pFilename + ".ini not found.");
		}
		~fileNotFoundException() {
			// expecting that the provided string will not need to be used elsewhere
			delete pFilename;
			delete pMsg;
		}
		const char* what() const noexcept {
			return pMsg->data();
		}
};

#endif // USEFUL_EXCEPTIONS_HPP
