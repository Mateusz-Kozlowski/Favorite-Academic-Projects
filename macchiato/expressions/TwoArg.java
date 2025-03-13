package expressions;

import instructions.Block;

public abstract class TwoArg extends Expression {
	protected Expression left;
	protected Expression right;

	public TwoArg(Expression left, Expression right) {
		this.left = left;
		this.right = right;
	}

	@Override
	public abstract int evaluate(Block[] metaBlocks) throws Exception;

	@Override
	public abstract TwoArg deepCopy();

	@Override
	public abstract String toString();
}
