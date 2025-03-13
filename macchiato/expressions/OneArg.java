package expressions;

import instructions.Block;

public abstract class OneArg extends Expression {
	@Override
	public abstract int evaluate(Block[] metaBlocks);

	@Override
	public abstract OneArg deepCopy();

	@Override
	public abstract String toString();
}
