#ifndef AST_H
# define AST_H

typedef struct s_ast_node	t_ast_node;
typedef	enum	e_node_type	t_node_type;

t_ast_node	*new_cmd_node(void);
t_ast_node	*new_op_node(t_node_type type, t_ast_node *ledt, t_ast_node *right);

#endif
