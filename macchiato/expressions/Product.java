package expressions;

import instructions.Block;

public class Product extends TwoArg {
	public Product(Expression left, Expression right) {
		super(left, right);
	}

	@Override
	public int evaluate(Block[] metaBlocks) throws Exception {
		return left.evaluate(metaBlocks) * right.evaluate(metaBlocks);
	}

	@Override
	public String toString() {
		return '(' + left.toString() + " * " + right.toString() + ')';
	}

	@Override
	public TwoArg deepCopy() { return new Product(left.deepCopy(), right.deepCopy()); }

	public static Product of(Expression left, Expression right) {
		return new Product(left, right);
	}
}
