import Base.eval

# `expr` is the Julia expression, and args is an iterable of variable names and
# their values.
function y(expr_str, args...)
    args_dict = Dict([(Symbol(args[i]), args[i + 1]) for i in 1:2:length(args)])
    expr = Meta.parse(expr_str)
    prepare_expr(expr, args_dict)

    return eval(expr)
end

# Prepare an expression for evaluation by replacing the symbols in it with their
# appropriate values (from the dictionary).
function prepare_expr(expr, args_dict)
    if expr isa Symbol && haskey(args_dict, expr)
        return args_dict[expr]
    elseif expr isa Expr
        for i in eachindex(expr.args)
            expr.args[i] = prepare_expr(expr.args[i], args_dict)
        end
    end

    return expr
end
