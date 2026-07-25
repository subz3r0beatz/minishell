/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ast.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fldumas- <fldumas-@student.42angouleme.fr  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 16:02:47 by fldumas-          #+#    #+#             */
/*   Updated: 2026/07/25 18:31:15 by fldumas-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AST_H
# define AST_H

typedef struct s_ast_node	t_ast_node;
typedef enum e_node_type	t_node_type;

int			new_cmd_node(t_ast_node **node);
t_ast_node	*new_op_node(t_node_type type, t_ast_node *left, t_ast_node *right);
t_ast_node	*free_ast(t_ast_node *ast);

#endif
