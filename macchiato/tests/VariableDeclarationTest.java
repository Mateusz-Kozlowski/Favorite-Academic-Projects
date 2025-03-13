package tests;

import builders.BlockBuilder;
import expressions.Literal;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class VariableDeclarationTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareVariable('a', Literal.of(5))
				.declareVariable('b', Literal.of(6))
				.build();

			int res = block.startProgram(false);

			assertEquals(res, 0);
			assertEquals(5, block.getVariableValue('a', new Block[] {block}));
			assertEquals(6, block.getVariableValue('b', new Block[] {block}));
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}
