// TODO: Don't forget to add 42 header !

/**
 * @file ASTNode.cpp
 * @brief
 */

#include "AST.hpp"
#include <cstddef>

namespace abnf
{
namespace ast
{

/**
 * @brief 
 */
ASTNode::ASTNode(e_RuleType initType, const std::string &initValue, int initMinRep, int initMaxRep)
: type(initType), value(initValue), minRep(initMinRep), maxRep(initMaxRep) {}

/**
 * @brief 
 */
ASTNode::~ASTNode()
{
	std::vector<ASTNode *>::iterator it;
	for (it = this->children.begin(); it != this->children.end(); it++)
		delete *it;
}

/**
 * @brief 
 *
 * @param rhs 
 */
ASTNode::ASTNode(const ASTNode &rhs) : type(rhs.type), value(rhs.value), minRep(rhs.minRep), maxRep(rhs.maxRep)
{
	this->copyChildren(rhs);
}

/**
 * @brief 
 *
 * @param rhs 
 * @return 
 */
ASTNode	&ASTNode::operator=(const ASTNode &rhs)
{
	if (this != &rhs)
	{
		this->type = rhs.type;
		this->value = rhs.value;
		this->copyChildren(rhs);
		this->minRep = rhs.minRep;
		this->maxRep = rhs.maxRep;
	}

	return (*this);
}

/**
 * @brief 
 *
 * @param rhs 
 */
void	ASTNode::copyChildren(const ASTNode &rhs)
{
	for (std::size_t i = 0; i < rhs.children.size(); i ++)
		this->children.push_back(new ASTNode (*rhs.children[i]));
}

/**
 * @brief 
 *
 * @return 
 */
ASTNode::e_RuleType	ASTNode::getType() const
{
	return (this->type);
}

/**
 * @brief 
 *
 * @return 
 */
const std::string	&ASTNode::getValue() const
{
	return (this->value);
}

/**
 * @brief 
 *
 * @return 
 */
int			ASTNode::getMinRep() const
{
	return (this->minRep);
}

/**
 * @brief 
 *
 * @return 
 */
int			ASTNode::getMaxRep() const
{
	return (this->maxRep);
}

/**
 * @brief 
 *
 * @return 
 */
std::size_t	ASTNode::getChildrenSize() const
{
	return (this->children.size());
}

/**
 * @brief 
 *
 * @return 
 */
const std::vector<ASTNode *>	&ASTNode::getChildren() const
{
	return (this->children);
}

/**
 * @brief 
 *
 * @param child 
 */
void	ASTNode::addChild(ASTNode *child)
{
	this->children.push_back(child);
}

/**
 * @brief 
 *
 * @param i 
 * @return 
 */
ASTNode	*ASTNode::getChild(std::size_t i) const
{
	return (this->children.at(i));
}

} // !ast
} // !abnf
