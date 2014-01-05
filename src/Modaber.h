/* In the name of God  */

#ifndef MODABER_H_
#define MODABER_H_


namespace mdbr{

class Modaber {
protected:


	void instantiation(char *domainFile, char *problemFile);

	virtual void initialization(char *domainFilePath, char *problemFilePath);

	virtual bool tryToSolve() = 0;


public:

	Modaber(){}

	virtual ~Modaber();

};

} /* namespace mdbr */

#endif /* MODABER_H_ */
