package tests;

import builders.BlockBuilder;
import expressions.Literal;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class PrintTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.print(Literal.of(5))
				.print(Literal.of(-7))
				.build();

			int res = block.startProgram(false);

			assertEquals(res, 0);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}