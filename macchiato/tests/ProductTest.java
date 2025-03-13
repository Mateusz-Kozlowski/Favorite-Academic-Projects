package tests;

import instructions.Block;
import expressions.Literal;
import expressions.Product;
import org.junit.jupiter.api.Assertions;

import static org.junit.jupiter.api.Assertions.*;

class ProductTest {
	@org.junit.jupiter.api.Test
	void test() {
		try {
			Assertions.assertEquals(
				6,
				Product.of(Literal.of(1), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				0,
				Product.of(Literal.of(0), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				36,
				Product.of(Literal.of(6), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				108,
				Product.of(Literal.of(18), Literal.of(6)).evaluate(new Block[] {})
			);
			Assertions.assertEquals(
				-48,
				Product.of(Literal.of(-8), Literal.of(6)).evaluate(new Block[] {})
			);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}