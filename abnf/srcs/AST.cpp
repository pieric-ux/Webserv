// TODO: Don't forget to add 42 header !

/**
 * @file AST.cpp
 * @brief
 */

#include "AST.hpp"

namespace abnf
{
namespace ast
{

/**
 * @brief 
 */
AST::AST() : root(NULL) {}

/**
 * @brief 
 */
AST::~AST()
{
	delete this->root;
}

/**
 * @brief 
 *
 * @param rhs 
 */
AST::AST(const AST &rhs)
{
	(void)rhs;
}

/**
 * @brief 
 *
 * @param rhs 
 * @return 
 */
AST	&AST::operator=(const AST &rhs)
{
	(void)rhs;
	return (*this);
}

/**
 * @brief 
 *
 * @return 
 */
AST	&AST::getAST()
{
	static AST	instance;

	return (instance);
}

/**
 * @brief 
 *
 * @return 
 */
ASTNode *AST::getRoot() const
{
	return (this->root);
}

/**
 * @brief 
 *
 * @param node 
 */
void AST::setRoot(ASTNode *node)
{
	this->root = node;
}

} // !ast
} // !abnf
