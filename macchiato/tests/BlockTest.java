package tests;

import builders.BlockBuilder;
import instructions.Block;

import static org.junit.jupiter.api.Assertions.*;

class BlockTest {

	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.block(new BlockBuilder()
					.build()
				)
				.build();

			int res = block.startProgram(false);

			assertEquals(0, res);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}