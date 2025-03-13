package expressions;

import instructions.Block;

public abstract class Expression {
	public abstract int evaluate(Block[] metaBlocks) throws Exception;

	public abstract Expression deepCopy();

	@Override
	public abstract String toString();
}
