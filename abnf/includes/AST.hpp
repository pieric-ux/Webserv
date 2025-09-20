// TODO: Don't forget to add 42 header !

/**
 * @file AST.hpp
 * @brief
 */

#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>

namespace abnf {
namespace ast {

/**
 * @class ASTNode
 * @brief 
 *
 */
class ASTNode
{
	public:
		/**
		 * @enum e_RuleType
		 * @brief 
		 */
		enum e_RuleType {
			RULE,			// rule = anotherrule 
			TERMINAL,		// coreRules or literal ALHPA %x47.4D.54
			COMMENT,		// ; ...
			RANGE,			// %x00-%xFF
			REPETITION,		// 1*element
			GROUPING,		// ( )
			OPTIONAL,		// [ ]
			CONCATENATION,	// LWSP implicit
			ALTERNATIVE,	// "/"
			INCREMENTAL_ALT	// "=/"
		};
		/**
		 * @typedef t_RuleType
		 * @brief Alias for the enum `e_RuleType`
		 */

		ASTNode(e_RuleType type, const std::string &value, int min_rep, int max_rep);
		~ASTNode();

		ASTNode(const ASTNode &rhs);
		ASTNode	&operator=(const ASTNode &rhs);

		e_RuleType						getType() const;
		const std::string				&getValue() const;
		int								getMinRep() const;
		int								getMaxRep() const;
		std::size_t						getChildrenSize() const;
		const std::vector<ASTNode *>	&getChildren() const;

		void							addChild(ASTNode *child);
		ASTNode							*getChild(std::size_t i) const;

	private:
		e_RuleType					type;
		std::string					value;
		std::vector<ASTNode *>		children;
		int							minRep; // minimum repetitions
		int							maxRep; // maximum repetitions (-1 = infinity)
	
		void	copyChildren(const ASTNode &rhs);

};

/**
 * @class AST
 * @brief 
 *
 */
class AST
{
	public:
		static AST	&getAST();

		ASTNode	*getRoot() const;
		void	setRoot(ASTNode *node);

	private:
		AST();
		~AST();

		AST(const AST &rhs);
		AST	&operator=(const AST &rhs);

		ASTNode	*root;
};

} // !namespace ast
} // !namespace abnf 

#endif // !AST_HPP
